#include <gtest/gtest.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-cps.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>

namespace fs = std::filesystem;

// Test data for incremental parsing
struct IncrementalTestCase {
    std::string initial_content;
    std::string modified_content;
    uint32_t start_byte;
    uint32_t old_end_byte;
    uint32_t new_end_byte;
    std::string description;
};

// Test fixture for CPS parser tests
class CPSParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = ts_parser_new();
        const TSLanguage *language = tree_sitter_cps();
        ts_parser_set_language(parser, language);
    }

    void TearDown() override {
        ts_parser_delete(parser);
    }

    std::string readFile(const fs::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + path.string());
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }

    std::string parseAndGetSExpr(const std::string& content) {
        TSTree *tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.length());
        if (!tree) {
            return "";
        }

        TSNode root_node = ts_tree_root_node(tree);
        char *string = ts_node_string(root_node);
        std::string result;
        if (string) {
            result = string;
            free(string);
        }
        ts_tree_delete(tree);

        return result;
    }

    // Incremental parsing: parse with existing tree
    std::string parseIncrementally(TSTree *old_tree, const std::string& new_content,
                                 uint32_t start_byte, uint32_t old_end_byte, uint32_t new_end_byte) {
        TSInputEdit edit;
        edit.start_byte = start_byte;
        edit.old_end_byte = old_end_byte;
        edit.new_end_byte = new_end_byte;
        edit.start_point = {0, start_byte};  // Simplified: assume single line
        edit.old_end_point = {0, old_end_byte};
        edit.new_end_point = {0, new_end_byte};

        ts_tree_edit(old_tree, &edit);

        TSTree *new_tree = ts_parser_parse_string(parser, old_tree,
                                                new_content.c_str(), new_content.length());

        TSNode root_node = ts_tree_root_node(new_tree);
        char *string = ts_node_string(root_node);
        std::string result;
        if (string) {
            result = string;
            free(string);
        }

        ts_tree_delete(old_tree);
        ts_tree_delete(new_tree);

        return result;
    }

    // Normalize S-expression by removing extra whitespace
    std::string normalizeSExpression(const std::string& sexpr) {
        std::string result = sexpr;

        // Remove extra whitespace
        std::regex whitespace_regex("\\s+");
        result = std::regex_replace(result, whitespace_regex, " ");

        // Trim leading/trailing whitespace
        result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), result.end());

        return result;
    }

    TSParser *parser;
};

// Basic parsing test - DISABLED until grammar is complete
TEST_F(CPSParserTest, DISABLED_BasicParsing) {
    std::string input = "namespace test {}";
    std::string actual_sexpr = parseAndGetSExpr(input);
    ASSERT_FALSE(actual_sexpr.empty()) << "Failed to parse: " << input;

    // Should contain namespace and identifier
    EXPECT_NE(actual_sexpr.find("namespace"), std::string::npos);
    EXPECT_NE(actual_sexpr.find("test"), std::string::npos);
}

// Incremental parsing test - DISABLED until grammar is complete
TEST_F(CPSParserTest, DISABLED_IncrementalParsing) {
    // Initial content
    std::string initial_content = "namespace test {\n  // comment\n}";
    TSTree *initial_tree = ts_parser_parse_string(parser, nullptr,
                                                initial_content.c_str(), initial_content.length());
    ASSERT_TRUE(initial_tree != nullptr) << "Failed to parse initial content";

    // Get initial parse tree
    std::string initial_sexpr = parseAndGetSExpr(initial_content);
    ASSERT_FALSE(initial_sexpr.empty()) << "Failed to get initial S-expression";

    // Modify content: insert a function inside the namespace
    std::string modified_content = "namespace test {\n  void func() {}\n  // comment\n}";
    std::string expected_insertion = "void func() {}";

    // The edit: insert "  void func() {}\n" at position 16 (after "namespace test {\n")
    // Old end: position 16, new end: position 16 + length of insertion
    uint32_t start_byte = 16;  // After "namespace test {\n"
    uint32_t old_end_byte = 16;
    uint32_t new_end_byte = 16 + expected_insertion.length() + 1;  // +1 for newline

    std::string new_sexpr = parseIncrementally(initial_tree, modified_content,
                                             start_byte, old_end_byte, new_end_byte);

    ASSERT_FALSE(new_sexpr.empty()) << "Failed to parse modified content incrementally";

    // Verify the new content contains the function
    EXPECT_NE(new_sexpr.find("func"), std::string::npos) << "Function not found in incrementally parsed tree";
    EXPECT_NE(new_sexpr.find("void"), std::string::npos) << "Function return type not found";

    // Verify the structure is maintained
    EXPECT_NE(new_sexpr.find("namespace"), std::string::npos) << "Namespace structure lost";
    EXPECT_NE(new_sexpr.find("test"), std::string::npos) << "Namespace name lost";
}

// Test incremental parsing with deletion - DISABLED until grammar is complete
TEST_F(CPSParserTest, DISABLED_IncrementalParsingDeletion) {
    // Initial content with extra content to delete
    std::string initial_content = "namespace test {\n  int x = 42;\n  void func() {}\n}";
    TSTree *initial_tree = ts_parser_parse_string(parser, nullptr,
                                                initial_content.c_str(), initial_content.length());
    ASSERT_TRUE(initial_tree != nullptr) << "Failed to parse initial content";

    // Modify content: remove the variable declaration
    std::string modified_content = "namespace test {\n  void func() {}\n}";
    std::string removed_content = "\n  int x = 42;";

    // The edit: remove "  int x = 42;\n" starting at position 18
    uint32_t start_byte = 18;  // Position of "  int x = 42;\n"
    uint32_t old_end_byte = start_byte + removed_content.length();
    uint32_t new_end_byte = start_byte;

    std::string new_sexpr = parseIncrementally(initial_tree, modified_content,
                                             start_byte, old_end_byte, new_end_byte);

    ASSERT_FALSE(new_sexpr.empty()) << "Failed to parse modified content incrementally";

    // Verify the variable is gone
    EXPECT_EQ(new_sexpr.find("int"), std::string::npos) << "Variable declaration still present after deletion";
    EXPECT_EQ(new_sexpr.find("x"), std::string::npos) << "Variable name still present after deletion";
    EXPECT_EQ(new_sexpr.find("42"), std::string::npos) << "Variable value still present after deletion";

    // Verify the function is still there
    EXPECT_NE(new_sexpr.find("func"), std::string::npos) << "Function lost during deletion";
    EXPECT_NE(new_sexpr.find("void"), std::string::npos) << "Function return type lost during deletion";
}

// Test incremental parsing performance benefit - DISABLED until grammar is complete
TEST_F(CPSParserTest, DISABLED_IncrementalParsingEfficiency) {
    // Create a larger document
    std::string large_content;
    for (int i = 0; i < 100; ++i) {
        large_content += "namespace ns" + std::to_string(i) + " {\n";
        large_content += "  void func" + std::to_string(i) + "() {\n";
        large_content += "    // some code\n";
        large_content += "  }\n";
        large_content += "}\n\n";
    }

    // Parse initial large document
    TSTree *initial_tree = ts_parser_parse_string(parser, nullptr,
                                                large_content.c_str(), large_content.length());
    ASSERT_TRUE(initial_tree != nullptr) << "Failed to parse large initial content";

    // Make a small edit at the end
    std::string modified_content = large_content + "\n// end comment\n";

    // Measure that incremental parsing works (we can't easily measure time in unit tests,
    // but we can verify correctness)
    uint32_t start_byte = large_content.length();
    uint32_t old_end_byte = start_byte;
    uint32_t new_end_byte = modified_content.length();

    std::string new_sexpr = parseIncrementally(initial_tree, modified_content,
                                             start_byte, old_end_byte, new_end_byte);

    ASSERT_FALSE(new_sexpr.empty()) << "Failed incremental parsing of large document";

    // Verify the comment was added
    EXPECT_NE(new_sexpr.find("end"), std::string::npos) << "End comment not found in incrementally parsed tree";
    EXPECT_NE(new_sexpr.find("comment"), std::string::npos) << "Comment not found in incrementally parsed tree";

    // Verify structure is maintained (should still have many namespaces)
    size_t namespace_count = 0;
    size_t pos = 0;
    while ((pos = new_sexpr.find("namespace", pos)) != std::string::npos) {
        namespace_count++;
        pos += 9;
    }
    EXPECT_GE(namespace_count, 95) << "Too many namespaces lost during incremental parsing";
}

// Test cases for incremental parsing
std::vector<IncrementalTestCase> GetIncrementalTestCases() {
    return {
        {
            "namespace test {\n  // comment\n}",  // initial
            "namespace test {\n  void func() {}\n  // comment\n}",  // modified
            16, 16, 33,  // edit range: insert at position 16, "  void func() {}\n"
            "Insert function in namespace"
        },
        {
            "namespace test {\n  int x = 42;\n  void func() {}\n}",  // initial
            "namespace test {\n  void func() {}\n}",  // modified (remove variable)
            18, 32, 18,  // edit range: remove "  int x = 42;\n"
            "Remove variable declaration"
        },
        {
            "namespace a {\n}",  // initial
            "namespace abc {\n}",  // modified
            10, 11, 13,  // edit range: change "a" to "abc"
            "Extend namespace name"
        }
    };
}

// Custom test name generator for incremental tests
static std::string PrintIncrementalTestName(const ::testing::TestParamInfo<IncrementalTestCase>& info) {
    // Replace spaces and special chars with underscores for valid test names
    std::string name = info.param.description;
    std::replace(name.begin(), name.end(), ' ', '_');
    std::replace(name.begin(), name.end(), '(', '_');
    std::replace(name.begin(), name.end(), ')', '_');
    return name;
}

// Test fixture for incremental parsing tests
class CPSIncrementalTest : public CPSParserTest,
                          public ::testing::WithParamInterface<IncrementalTestCase> {
};

// Parameterized test for incremental parsing scenarios
TEST_P(CPSIncrementalTest, IncrementalParse) {
    auto test_case = GetParam();

    // Parse initial content
    TSTree *initial_tree = ts_parser_parse_string(parser, nullptr,
                                                test_case.initial_content.c_str(),
                                                test_case.initial_content.length());
    ASSERT_TRUE(initial_tree != nullptr) << "Failed to parse initial content: " << test_case.description;

    // Perform incremental parsing
    std::string new_sexpr = parseIncrementally(initial_tree,
                                             test_case.modified_content,
                                             test_case.start_byte,
                                             test_case.old_end_byte,
                                             test_case.new_end_byte);

    ASSERT_FALSE(new_sexpr.empty()) << "Failed incremental parsing: " << test_case.description;

    // Verify the tree structure is valid (contains expected elements)
    EXPECT_NE(new_sexpr.find("namespace"), std::string::npos) << "Namespace lost in: " << test_case.description;
}

// Instantiate the parameterized test with incremental test cases
INSTANTIATE_TEST_SUITE_P(
    CPSIncrementalTests,
    CPSIncrementalTest,
    ::testing::ValuesIn(GetIncrementalTestCases()),
    PrintIncrementalTestName
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
