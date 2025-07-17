#include "../src/compiler-test-suite.h"
#include <iostream>

int main() {
    std::cout << "C-- Compiler Comprehensive Test Suite\n";
    std::cout << "====================================\n\n";
    
    CompilerTestSuite test_suite;
    test_suite.run_all_tests();
    
    // Generate test reports
    test_suite.generate_test_report("html", "test_report.html");
    test_suite.generate_test_report("json", "test_report.json");
    
    std::cout << "\nTest reports generated:\n";
    std::cout << "  - test_report.html\n";
    std::cout << "  - test_report.json\n";
    
    return test_suite.get_exit_code();
}
