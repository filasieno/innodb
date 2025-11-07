#include <gtest/gtest.h>
#include <tree_sitter/api.h>
#include <tree-sitter-cps.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>

namespace fs = std::filesystem;

// Function to get all test file names
std::vector<std::string> GetTestFiles() {
    // Input files are in test/data/input
    fs::path input_dir = fs::current_path() / "test" / "data" / "input";

    std::vector<std::string> test_files;
    if (fs::exists(input_dir)) {
        for (const auto& entry : fs::directory_iterator(input_dir)) {
            if (entry.path().extension() == ".cps") {
                test_files.push_back(entry.path().stem().string());
            }
        }
    }
    std::sort(test_files.begin(), test_files.end());
    return test_files;
}

class CPSParserTest : public ::testing::TestWithParam<std::string> {
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

    // Normalize S-expression by removing extra whitespace and standardizing format
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


// Parameterized test that runs for each .cps file in data/input/
// Each file becomes its own test with the filename as the test name
TEST_P(CPSParserTest, ParseFile) {
    std::string filename = GetParam();

    // Input and expected are read from test/data
    fs::path input_dir = fs::current_path() / "test" / "data" / "input";
    fs::path expected_dir = fs::current_path() / "test" / "data" / "expected";

    // Actual results are generated in build folder
    fs::path actual_dir = fs::current_path() / "build" / "actual";

    // Check if directories exist
    ASSERT_TRUE(fs::exists(input_dir)) << "Input directory does not exist: " << input_dir;
    ASSERT_TRUE(fs::exists(expected_dir)) << "Expected directory does not exist: " << expected_dir;

    // Create actual directory if it doesn't exist
    if (!fs::exists(actual_dir)) {
        fs::create_directory(actual_dir);
    }

    fs::path input_file = input_dir / (filename + ".cps");
    fs::path expected_file = expected_dir / (filename + ".expected");

    // Read input file
    ASSERT_TRUE(fs::exists(input_file)) << "Input file does not exist: " << input_file;
    std::string content = readFile(input_file);

    // Parse the content and get S-expression
    std::string actual_sexpr = parseAndGetSExpr(content);
    ASSERT_FALSE(actual_sexpr.empty()) << "Failed to parse: " << filename;

    // Save actual output for debugging
    fs::path actual_file = actual_dir / (filename + ".actual");
    std::ofstream actual_out(actual_file);
    actual_out << actual_sexpr;
    actual_out.close();

    // Read expected S-expression
    ASSERT_TRUE(fs::exists(expected_file)) << "Expected file does not exist: " << expected_file;
    std::string expected_content = readFile(expected_file);

    // Normalize both S-expressions for comparison
    std::string normalized_actual = normalizeSExpression(actual_sexpr);
    std::string normalized_expected = normalizeSExpression(expected_content);

    // Compare the S-expressions - this will show which specific file failed
    EXPECT_EQ(normalized_expected, normalized_actual)
        << "S-expression mismatch for: " << filename << std::endl
        << "Expected: " << normalized_expected << std::endl
        << "Actual: " << normalized_actual << std::endl
        << "Actual output saved to: " << actual_file;
}

// Custom test name generator that returns the filename
static std::string PrintTestName(const ::testing::TestParamInfo<std::string>& info) {
    return info.param;
}

// Instantiate the parameterized test with all discovered test files
INSTANTIATE_TEST_SUITE_P(
    CPSParserTests,
    CPSParserTest,
    ::testing::ValuesIn(GetTestFiles()),
    PrintTestName
);


