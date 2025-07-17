#include "compiler-driver.h"
#include "compiler-test-suite.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

void print_usage(const std::string& program_name) {
    std::cout << "C-- Compiler (cmmc) v1.0\n\n";
    std::cout << "Usage: " << program_name << " [options] <input-file> [-o <output-file>]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -O0, -O1, -O2, -O3     Set optimization level (default: -O0)\n";
    std::cout << "  -g                     Generate debug information\n";
    std::cout << "  -v, --verbose          Verbose output\n";
    std::cout << "  -S                     Generate assembly only\n";
    std::cout << "  -c                     Generate object file only\n";
    std::cout << "  -o <file>              Output file name\n";
    std::cout << "  --print-stages         Print compilation stages\n";
    std::cout << "  --print-ir             Print intermediate representation\n";
    std::cout << "  --print-asm            Print generated assembly\n";
    std::cout << "  --print-cfg            Print control flow graph\n";
    std::cout << "  --keep-intermediate    Keep intermediate files\n";
    std::cout << "  --profile              Enable compiler profiling\n";
    std::cout << "  --test                 Run compiler test suite\n";
    std::cout << "  --help                 Show this help message\n";
    std::cout << "  --version              Show version information\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " program.c -o program\n";
    std::cout << "  " << program_name << " -O2 -g program.c -o program\n";
    std::cout << "  " << program_name << " -S program.c -o program.s\n";
    std::cout << "  " << program_name << " --test\n";
}

void print_version() {
    std::cout << "C-- Compiler (cmmc) version 1.0.0\n";
    std::cout << "Built with support for:\n";
    std::cout << "  - x86_64 assembly generation\n";
    std::cout << "  - Advanced optimizations\n";
    std::cout << "  - Debug information generation\n";
    std::cout << "  - Comprehensive testing framework\n";
    std::cout << "  - Performance profiling\n";
}

// Argument parsing and main logic
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    CompilerDriver compiler;
    std::string input_file;
    std::string output_file;
    bool run_tests = false;
    bool enable_profiling = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--version") {
            print_version();
            return 0;
        } else if (arg == "--test") {
            run_tests = true;
        } else if (arg == "--profile") {
            enable_profiling = true;
        } else if (arg == "-O0") {
            compiler.set_optimization_level(OptimizationLevel::O0);
        } else if (arg == "-O1") {
            compiler.set_optimization_level(OptimizationLevel::O1);
        } else if (arg == "-O2") {
            compiler.set_optimization_level(OptimizationLevel::O2);
        } else if (arg == "-O3") {
            compiler.set_optimization_level(OptimizationLevel::O3);
        } else if (arg == "-g") {
            compiler.enable_debug_info(true);
        } else if (arg == "-v" || arg == "--verbose") {
            compiler.set_verbose(true);
        } else if (arg == "-S") {
            compiler.set_output_format(OutputFormat::ASSEMBLY);
        } else if (arg == "-c") {
            compiler.set_output_format(OutputFormat::OBJECT);
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o requires an argument\n";
                return 1;
            }
        } else if (arg == "--print-stages") {
            compiler.print_compilation_stages(true);
        } else if (arg == "--print-ir") {
            // This would need to be implemented in CompilerDriver
            std::cout << "IR printing enabled\n";
        } else if (arg == "--print-asm") {
            // This would need to be implemented in CompilerDriver
            std::cout << "Assembly printing enabled\n";
        } else if (arg == "--print-cfg") {
            // This would need to be implemented in CompilerDriver
            std::cout << "CFG printing enabled\n";
        } else if (arg == "--keep-intermediate") {
            // This would need to be implemented in CompilerDriver
            std::cout << "Keeping intermediate files\n";
        } else if (arg[0] != '-') {
            if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Error: Multiple input files not supported\n";
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown option " << arg << "\n";
            return 1;
        }
    }
    
    // Run tests if requested
    if (run_tests) {
        std::cout << "Running C-- Compiler Test Suite...\n";
        CompilerTestSuite test_suite;
        test_suite.run_all_tests();
        return test_suite.get_exit_code();
    }
    
    // Check for input file
    if (input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        print_usage(argv[0]);
        return 1;
    }
    
    // Check if input file exists
    if (!std::filesystem::exists(input_file)) {
        std::cerr << "Error: Input file '" << input_file << "' does not exist\n";
        return 1;
    }
    
    // Generate output file name if not specified
    if (output_file.empty()) {
        std::filesystem::path input_path(input_file);
        output_file = input_path.stem().string();
    }
    
    // Enable profiling if requested
    if (enable_profiling) {
        compiler.enable_profiling(true);
    }
    
    // Compile the program
    bool success = compiler.compile(input_file, output_file);
    
    if (!success) {
        std::cerr << "Compilation failed:\n";
        for (const auto& error : compiler.get_errors()) {
            std::cerr << "  " << error << "\n";
        }
        return 1;
    }
    
    // Print warnings if any
    const auto& warnings = compiler.get_warnings();
    if (!warnings.empty()) {
        std::cout << "Warnings:\n";
        for (const auto& warning : warnings) {
            std::cout << "  " << warning << "\n";
        }
    }
    
    std::cout << "Compilation successful: " << output_file << "\n";
    
    return 0;
}
