#include "compiler-test-suite.h"
#include "compiler-driver.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <set>
#include <filesystem>

CompilerTestSuite::CompilerTestSuite()
    : total_tests(0), passed_tests(0), failed_tests(0), skipped_tests(0), error_tests(0) {
    // Load or define all test cases
    this->generate_lexer_tests();
    this->generate_parser_tests();
    this->generate_semantic_tests();
    this->generate_codegen_tests();
    this->generate_integration_tests();
    this->generate_performance_tests();
    this->generate_edge_case_tests();
    this->generate_regression_tests();

    this->total_tests = static_cast<int>(this->test_cases.size());
}

void CompilerTestSuite::run_all_tests() {
    // Clear previous results and counters
    this->passed_tests = 0;
    this->failed_tests = 0;
    this->skipped_tests = 0;
    this->error_tests = 0;

    // Use actual count of tests run, not stored total_tests
    int actual_total = 0;

    std::cout << "Running C-- Compiler Test Suite\n";
    std::cout << "================================\n";

    for (const auto& test : this->test_cases) {
        // Skip disabled tests
        if (!test.enabled) {
            this->skipped_tests++;
            continue;
        }

        actual_total++;

        std::cout << "Running: " << test.name << "... ";

        CompilerDriver compiler;
        compiler.set_verbose(false);
        compiler.print_compilation_stages(false);

        // Write test source code to temp file
        std::string temp_file = "temp_test_input.cmm";
        std::ofstream fout(temp_file);
        fout << test.source_code;
        fout.close();

        bool success = compiler.compile(temp_file, "temp_out");

        TestResult result;
        std::ostringstream msg;

        if (test.should_compile) {
            if (success) {
                result = TestResult::PASSED;
                std::cout << "PASSED\n";
                this->passed_tests++;
            } else {
                result = TestResult::FAILED;
                msg << "Compilation failed but was expected to succeed.";
                std::cout << "FAILED\n";
                this->failed_tests++;
            }
        } else {
            if (!success) {
                result = TestResult::PASSED;
                std::cout << "PASSED\n";
                this->passed_tests++;

                // Check expected errors
                const auto& errors = compiler.get_errors();
                for (const std::string& expected_error : test.expected_errors) {
                    auto found = std::any_of(errors.begin(), errors.end(), [&](const std::string& actual){
                        return actual.find(expected_error) != std::string::npos;
                    });
                    if (!found) {
                        msg << "Missing expected error: " << expected_error << "\n";
                        result = TestResult::FAILED;
                        this->failed_tests++;
                    }
                }
            } else {
                result = TestResult::FAILED;
                msg << "Compilation succeeded but was expected to fail.";
                std::cout << "FAILED\n";
                this->failed_tests++;
            }
        }

        this->test_results[test.name] = {
            result,
            msg.str(),
            0.0,
            "", {}, {}
        };

        // Cleanup
        std::filesystem::remove(temp_file);
        std::filesystem::remove("temp_out");
    }

    this->total_tests = actual_total;

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Total:   " << this->total_tests << "\n";
    std::cout << "Passed:  " << this->passed_tests << "\n";
    std::cout << "Failed:  " << this->failed_tests << "\n";
    std::cout << "Skipped: " << this->skipped_tests << "\n";
    std::cout << "Errors:  " << this->error_tests << "\n";
    std::cout << "====================\n";
}

// Test execution and reporting
void CompilerTestSuite::generate_test_report(const std::string& format, const std::string& output_file) {
    if (format == "html") {
        this->generate_html_report(output_file);
    } else if (format == "json") {
        this->generate_json_report(output_file);
    } else if (format == "xml") {
        this->generate_xml_report(output_file);
    } else {
        throw std::invalid_argument("Invalid report format: " + format);
    }
}

// ---------------------- Reporting Formats ----------------------

void CompilerTestSuite::generate_json_report(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open JSON report file: " + filename);
    }

    file << "{\n";
    file << "  \"test_suite\": {\n";
    file << "    \"name\": \"C-- Compiler Tests\",\n";
    file << "    \"total_tests\": " << total_tests << ",\n";
    file << "    \"passed_tests\": " << passed_tests << ",\n";
    file << "    \"failed_tests\": " << failed_tests << ",\n";
    file << "    \"error_tests\": " << error_tests << ",\n";
    file << "    \"skipped_tests\": " << skipped_tests << ",\n";
    file << "    \"pass_rate\": " << std::fixed << std::setprecision(1) << get_pass_rate() << ",\n";
    file << "    \"execution_time\": " << std::fixed << std::setprecision(3) << get_total_execution_time() << ",\n";
    file << "    \"tests\": [\n";

    bool first = true;
    for (const auto& test : test_cases) {
        if (!first) file << ",\n";
        first = false;

        auto result_it = test_results.find(test.name);
        if (result_it != test_results.end()) {
            const auto& result = result_it->second;
            file << "      {\n";
            file << "        \"name\": \"" << test.name << "\",\n";
            file << "        \"category\": \"" << test.category << "\",\n";
            file << "        \"result\": \"";

            switch (result.result) {
                case TestResult::PASSED: file << "PASSED"; break;
                case TestResult::FAILED: file << "FAILED"; break;
                case TestResult::SKIPPED: file << "SKIPPED"; break;
                case TestResult::ERROR: file << "ERROR"; break;
            }

            file << "\",\n";
            file << "        \"execution_time\": " << std::fixed << std::setprecision(3) << result.execution_time << ",\n";
            file << "        \"message\": \"" << result.message << "\"\n";
            file << "      }";
        }
    }

    file << "\n    ]\n";
    file << "  }\n";
    file << "}\n";

    file.close();
}

void CompilerTestSuite::generate_xml_report(const std::string& output_file) {
    std::ofstream out(output_file);
    if (!out.is_open()) throw std::runtime_error("Failed to open file " + output_file);

    out << "<?xml version=\"1.0\"?>\n<testsuite name=\"C-- Compiler Test Suite\" ";
    out << "tests=\"" << this->total_tests << "\" ";
    out << "failures=\"" << this->failed_tests << "\" ";
    out << "errors=\"" << this->error_tests << "\">\n";

    for (const auto& [name, result] : this->test_results) {
        out << "  <testcase name=\"" << name << "\">";
        if (result.result == TestResult::FAILED || result.result == TestResult::ERROR) {
            out << "<failure>" << result.message << "</failure>";
        }
        out << "</testcase>\n";
    }

    out << "</testsuite>\n";
    out.close();
}

void CompilerTestSuite::generate_html_report(const std::string& output_file) {
    std::ofstream out(output_file);
    if (!out.is_open()) throw std::runtime_error("Failed to open file " + output_file);

    out << "<!DOCTYPE html>\n<html>\n<head>\n<title>C-- Compiler Test Report</title>\n</head>\n<body>\n";
    out << "<h1>C-- Test Summary</h1>\n";
    out << "<p>Total: " << this->total_tests << ", Passed: " << this->passed_tests
        << ", Failed: " << this->failed_tests << ", Errors: " << this->error_tests << "</p>\n";

    out << "<table border=\"1\">\n<tr><th>Test Name</th><th>Result</th><th>Message</th></tr>\n";
    for (const auto& [name, info] : this->test_results) {
        out << "<tr><td>" << name << "</td><td>"
            << (info.result == TestResult::PASSED ? "PASS" :
                info.result == TestResult::FAILED ? "FAIL" :
                info.result == TestResult::ERROR ? "ERROR" : "SKIPPED")
            << "</td><td>" << info.message << "</td></tr>\n";
    }
    out << "</table>\n</body>\n</html>\n";

    out.close();
}

int CompilerTestSuite::get_exit_code() const {
    return this->failed_tests > 0 || this->error_tests > 0 ? 1 : 0;
}

// Test generators
// --- Test generators with proper enabled field ---

void CompilerTestSuite::generate_lexer_tests() {
    // These tests should just verify compilation succeeds with valid programs
    test_cases.push_back({
        "lexer_basic_tokens",
        "Test basic token recognition",
        "int main(void) { int x; x = 42; return x; }",
        "",
        true,
        {},
        {},
        "lexer",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "lexer_keywords",
        "Test keyword recognition",
        "int main(void) { int x; if (x > 0) { return x; } else { return 0; } }",
        "",
        true,
        {},
        {},
        "lexer",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "lexer_operators",
        "Test operator recognition",
        "int main(void) { int x; int y; int z; x = 5; y = 3; z = x + y - 1 * 2 / 1; return z; }",
        "",
        true,
        {},
        {},
        "lexer",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "lexer_invalid_char",
        "Test invalid character handling",
        "int main(void) { int x; x = @; return x; }",
        "",
        false,
        {"Invalid character"},
        {},
        "lexer",
        2,
        5.0,
        {},
        true  // enabled
    });
}

void CompilerTestSuite::generate_parser_tests() {
    test_cases.push_back({
        "parser_function_declaration",
        "Test function declaration parsing",
        "int main(void) { return 0; }",
        "",
        true,
        {},
        {},
        "parser",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "parser_variable_declaration",
        "Test variable declaration parsing",
        "int main(void) { int x; int y; return 0; }",
        "",
        true,
        {},
        {},
        "parser",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "parser_expression",
        "Test expression parsing",
        "int main(void) { int x; x = 1 + 2 * 3; return x; }",
        "",
        true,
        {},
        {},
        "parser",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "parser_syntax_error",
        "Test syntax error handling",
        "int main(void) { int x return 0; }",
        "",
        false,
        {"Expected ';'"},
        {},
        "parser",
        2,
        5.0,
        {},
        true  // enabled
    });
}

void CompilerTestSuite::generate_semantic_tests() {
    test_cases.push_back({
        "semantic_type_checking",
        "Test basic type checking",
        "int main(void) { int x; x = 42; return x; }",
        "",
        true,
        {},
        {},
        "semantic",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "semantic_undefined_variable",
        "Test undefined variable detection",
        "int main(void) { x = 42; return x; }",
        "",
        false,
        {"Undefined variable"},
        {},
        "semantic",
        1,
        5.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "semantic_void_variable",
        "Test void variable error",
        "int main(void) { void x; return 0; }",
        "",
        false,
        {"void variable"},
        {},
        "semantic",
        1,
        5.0,
        {},
        true  // enabled
    });
}

void CompilerTestSuite::generate_codegen_tests() {
    test_cases.push_back({
        "codegen_arithmetic",
        "Test arithmetic code generation",
        "int main(void) { int x; x = 5 + 3; return x; }",
        "",
        true,
        {},
        {},
        "codegen",
        1,
        10.0,
        {},
        true  // enabled
    });

    test_cases.push_back({
        "codegen_control_flow",
        "Test control flow code generation",
        "int main(void) { int x; x = 10; if (x > 5) { x = x - 1; } return x; }",
        "",
        true,
        {},
        {},
        "codegen",
        1,
        10.0,
        {},
        true  // enabled
    });
}

void CompilerTestSuite::generate_integration_tests() {}
void CompilerTestSuite::generate_performance_tests() {}
void CompilerTestSuite::generate_edge_case_tests() {}
void CompilerTestSuite::generate_regression_tests() {}

double CompilerTestSuite::get_pass_rate() const {
    if (this->total_tests == 0) return 0.0;
    return (static_cast<double>(this->passed_tests) / this->total_tests) * 100.0;
}

double CompilerTestSuite::get_total_execution_time() const {
    using namespace std::chrono;
    return duration<double>(this->end_time - this->start_time).count();
}