#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>

enum class TestResult {
    PASSED,
    FAILED,
    SKIPPED,
    ERROR
};

struct TestCase {
    std::string name;
    std::string description;
    std::string source_code;
    std::string expected_output;
    bool should_compile;
    std::vector<std::string> expected_errors;
    std::vector<std::string> expected_warnings;
    std::string category;
    int priority;
    double timeout_seconds;
    std::map<std::string, std::string> metadata;
    bool enabled = true;  // Default to enabled
};

struct TestResult_Info {
    TestResult result;
    std::string message;
    double execution_time;
    std::string actual_output;
    std::vector<std::string> actual_errors;
    std::vector<std::string> actual_warnings;
};

class CompilerTestSuite {
private:
    std::vector<TestCase> test_cases;
    std::map<std::string, TestResult_Info> test_results;
    std::map<std::string, std::vector<TestCase>> test_categories;
    
    // Test statistics
    int total_tests;
    int passed_tests;
    int failed_tests;
    int skipped_tests;
    int error_tests;
    
    // Performance tracking
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
    
    // Test execution helpers
    TestResult_Info execute_test(const TestCase& test);
    bool compile_and_run(const std::string& source, const std::string& expected_output);
    bool check_compilation_errors(const std::vector<std::string>& actual_errors,
                                 const std::vector<std::string>& expected_errors);
    bool run_executable(const std::string& executable_path, std::string& output);
    
    // Test case generators
    void generate_lexer_tests();
    void generate_parser_tests();
    void generate_semantic_tests();
    void generate_codegen_tests();
    void generate_integration_tests();
    void generate_performance_tests();
    void generate_edge_case_tests();
    void generate_regression_tests();
    
    // Report generation
    void generate_html_report(const std::string& output_file);
    void generate_xml_report(const std::string& output_file);
    void generate_json_report(const std::string& output_file);
    
public:
    CompilerTestSuite();
    ~CompilerTestSuite() = default;
    
    // Test management
    void add_test_case(const TestCase& test);
    void remove_test_case(const std::string& name);
    void clear_test_cases();
    
    // Test execution
    void run_all_tests();
    void run_category_tests(const std::string& category);
    void run_single_test(const std::string& test_name);
    void run_lexer_tests();
    void run_parser_tests();
    void run_semantic_tests();
    void run_codegen_tests();
    void run_integration_tests();
    void run_performance_tests();
    void run_regression_tests();
    
    // Test filtering
    void set_test_filter(const std::string& pattern);
    void set_category_filter(const std::string& category);
    void set_priority_filter(int min_priority);
    
    // Results and reporting
    void print_test_summary();
    void print_detailed_results();
    void generate_test_report(const std::string& format, const std::string& output_file);
    
    // Test statistics
    int get_total_tests() const;
    int get_passed_tests() const;
    int get_failed_tests() const;
    double get_pass_rate() const;
    double get_total_execution_time() const;
    
    // Test utilities
    void load_tests_from_file(const std::string& filename);
    void save_tests_to_file(const std::string& filename);
    void validate_test_suite();
    
    // Continuous integration support
    bool run_ci_tests();
    void generate_ci_report();
    int get_exit_code() const;
};
