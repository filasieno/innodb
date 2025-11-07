#include <gtest/gtest.h>
#include <tree_sitter/api.h>
#include <tree_sitter_cps.h>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

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
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }

    bool parseFile(const std::string& content) {
        TSTree *tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.length());
        bool success = (tree != nullptr && !ts_tree_has_errors(tree));
        if (tree) {
            ts_tree_delete(tree);
        }
        return success;
    }

    TSParser *parser;
};

TEST_F(CPSParserTest, ParseTestFiles) {
    // Get the test data directory
    fs::path test_data_dir = fs::path(__FILE__).parent_path().parent_path() / "data";
    fs::path input_dir = test_data_dir / "input";
    fs::path expected_dir = test_data_dir / "expected";

    // Iterate through all input files
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() == ".cps") {
            std::string filename = entry.path().stem().string();
            fs::path input_file = entry.path();
            fs::path expected_file = expected_dir / (filename + ".expected");

            SCOPED_TRACE("Testing file: " + input_file.string());

            // Read input file
            ASSERT_TRUE(fs::exists(input_file)) << "Input file does not exist: " << input_file;
            std::string content = readFile(input_file);

            // Parse the content
            bool parse_success = parseFile(content);

            // Read expected result
            ASSERT_TRUE(fs::exists(expected_file)) << "Expected file does not exist: " << expected_file;
            std::string expected_content = readFile(expected_file);

            // Check if parsing should succeed based on expected file
            if (expected_content.find("PARSE_SUCCESS") != std::string::npos) {
                EXPECT_TRUE(parse_success) << "Expected parsing to succeed for: " << filename;
            } else if (expected_content.find("PARSE_FAIL") != std::string::npos) {
                EXPECT_FALSE(parse_success) << "Expected parsing to fail for: " << filename;
            }
        }
    }
}

TEST_F(CPSParserTest, NamespaceExample) {
    std::string code = R"(
        namespace my_namespace {
            fn add(a: int, b: int): int {
                return a + b;
            }
        }
    )";

    EXPECT_TRUE(parseFile(code)) << "Namespace example should parse successfully";
}

TEST_F(CPSParserTest, FunctionExample) {
    std::string code = R"(
        fn calculate(x: int, y: int): int {
            var result = x * 2;
            result = result + y;
            return result;
        }
    )";

    EXPECT_TRUE(parseFile(code)) << "Function example should parse successfully";
}

TEST_F(CPSParserTest, StructExample) {
    std::string code = R"(
        struct Point {
            x: int;
            y: int;
        }

        fn distance(p1: Point, p2: Point): int {
            var dx = p1.x - p2.x;
            var dy = p1.y - p2.y;
            return dx * dx + dy * dy;
        }
    )";

    EXPECT_TRUE(parseFile(code)) << "Struct example should parse successfully";
}

TEST_F(CPSParserTest, ExpressionExample) {
    std::string code = R"(
        fn test_expressions(a: int, b: int, c: int): int {
            var result = a + b * c;
            if (result > 10 && result < 100) {
                result = result << 1;
            } else {
                result = result >> 1;
            }
            return result;
        }
    )";

    EXPECT_TRUE(parseFile(code)) << "Expression example should parse successfully";
}
