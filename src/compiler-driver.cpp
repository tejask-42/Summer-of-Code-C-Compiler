#include "compiler-driver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <cstdlib>

CompilerDriver::CompilerDriver() 
    : profiler(std::make_unique<CompilerProfiler>()) {
    // Initialize all compiler components
    lexer = std::make_unique<Lexer>("");
    parser = std::make_unique<Parser>(std::vector<Token>{});
    analyzer = std::make_unique<SemanticAnalyzer>();
    ir_generator = std::make_unique<IRGenerator>(analyzer.get());
    optimizer = std::make_unique<IROptimizer>();
    advanced_optimizer = std::make_unique<AdvancedOptimizer>();
    code_gen = std::make_unique<AssemblyGenerator>("temp.s");
    debug_gen = std::make_unique<DebugInfoGenerator>();
}

CompilerDriver::~CompilerDriver() {
    cleanup_intermediate_files();
}

// Main compile and phase methods
bool CompilerDriver::compile(const std::string& source_file, const std::string& output_file) {
    if (options.verbose) {
        std::cout << "C-- Compiler v1.0" << std::endl;
        std::cout << "Compiling: " << source_file << " -> " << output_file << std::endl;
    }
    
    // Read source file
    std::ifstream file(source_file);
    if (!file.is_open()) {
        error_messages.push_back("Cannot open source file: " + source_file);
        return false;
    }
    
    std::string source_code((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    
    return compile_from_source(source_code, output_file);
}

bool CompilerDriver::compile_from_source(const std::string& source_code, const std::string& output_file) {
    clear_messages();
    
    if (options.verbose) {
        profiler->start_phase("total_compilation");
    }
    
    // Phase 1: Lexical Analysis
    if (!run_lexical_analysis(source_code)) {
        return false;
    }
    
    // Phase 2: Syntax Analysis (PARSE ONLY ONCE HERE)
    if (!run_syntax_analysis()) {
        return false;
    }
    
    // Phase 3: Semantic Analysis
    if (!run_semantic_analysis()) {
        return false;
    }
    
    // Phase 4: IR Generation
    if (!run_ir_generation()) {
        return false;
    }
    
    // Phase 5: Optimization
    if (!run_optimization()) {
        return false;
    }
    
    // Phase 6: Code Generation
    std::string assembly_file = get_temporary_filename(".s");
    if (!run_code_generation(assembly_file)) {
        return false;
    }
    
    // Phase 7: Assembly and Linking
    if (options.output_format == OutputFormat::EXECUTABLE) {
        if (!run_assembly_and_linking(assembly_file, output_file)) {
            return false;
        }
    } else {
        // Just copy assembly file to output
        std::filesystem::copy_file(assembly_file, output_file);
    }
    
    if (options.verbose) {
        profiler->end_phase("total_compilation");
        print_performance_report();
    }
    
    if (!options.keep_intermediate) {
        cleanup_intermediate_files();
    }
    
    return true;
}

bool CompilerDriver::run_lexical_analysis(const std::string& source) {
    if (options.verbose) {
        profiler->start_phase("lexical_analysis");
    }
    
    try {
        lexer = std::make_unique<Lexer>(source);
        auto tokens = lexer->tokenize();
        
        if (options.print_stages) {
            std::cout << "Lexical Analysis: Generated " << tokens.size() << " tokens" << std::endl;
        }
        
        parser = std::make_unique<Parser>(tokens);
        
        if (options.verbose) {
            profiler->end_phase("lexical_analysis");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("Lexical analysis failed: " + std::string(e.what()));
        return false;
    }
}

bool CompilerDriver::run_syntax_analysis() {
    if (options.verbose) {
        profiler->start_phase("syntax_analysis");
    }
    
    try {
        ast = std::unique_ptr<Program>(dynamic_cast<Program*>(parser->parse_program().release()));
        if (!ast) {
            error_messages.push_back("Syntax analysis failed: No AST generated");
            return false;
        }
        
        if (options.print_stages) {
            std::cout << "Syntax Analysis: AST generated successfully" << std::endl;
        }
        
        if (options.verbose) {
            profiler->end_phase("syntax_analysis");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("Syntax analysis failed: " + std::string(e.what()));
        return false;
    }
}

bool CompilerDriver::run_semantic_analysis() {
    if (options.verbose) {
        profiler->start_phase("semantic_analysis");
    }
    
    try {
        // Don't call parse_program again! Use stored AST
        if (!ast) {
            error_messages.push_back("Semantic analysis failed: No AST");
            return false;
        }
        
        Program* program = dynamic_cast<Program*>(ast.get());
        if (!program) {
            error_messages.push_back("Semantic analysis failed: Invalid AST");
            return false;
        }
        
        bool success = analyzer->analyze(*program);
        
        if (!success) {
            for (const auto& error : analyzer->get_error_collector().get_errors()) {
                error_messages.push_back(error.format_error());
            }
            return false;
        }
        
        if (options.print_stages) {
            std::cout << "Semantic Analysis: Passed" << std::endl;
        }
        
        if (options.verbose) {
            profiler->end_phase("semantic_analysis");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("Semantic analysis failed: " + std::string(e.what()));
        return false;
    }
}

bool CompilerDriver::run_ir_generation() {
    if (options.verbose) {
        profiler->start_phase("ir_generation");
    }
    
    try {
        // Use stored AST, don't parse again
        if (!ast) {
            error_messages.push_back("IR generation failed: No AST");
            return false;
        }
        
        Program* program = dynamic_cast<Program*>(ast.get());
        auto ir_code = ir_generator->generate(*program);
        
        if (options.print_ir) {
            std::cout << "Generated IR:" << std::endl;
            ir_generator->print_ir();
        }
        
        if (options.print_stages) {
            std::cout << "IR Generation: Generated " << ir_code.size() << " instructions" << std::endl;
        }
        
        if (options.verbose) {
            profiler->end_phase("ir_generation");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("IR generation failed: " + std::string(e.what()));
        return false;
    }
}

bool CompilerDriver::run_optimization() {
    if (options.verbose) {
        profiler->start_phase("optimization");
    }
    
    try {
        // Use stored AST, don't parse again
        if (!ast) {
            error_messages.push_back("Optimization failed: No AST");
            return false;
        }
        
        Program* program = dynamic_cast<Program*>(ast.get());
        auto ir_code = ir_generator->generate(*program);
        
        // Apply optimizations based on level
        if (options.opt_level >= OptimizationLevel::O1) {
            ir_code = optimizer->optimize(ir_code);
        }
        
        if (options.opt_level >= OptimizationLevel::O2) {
            advanced_optimizer->apply_dataflow_optimizations(ir_code);
        }
        
        if (options.opt_level >= OptimizationLevel::O3) {
            advanced_optimizer->apply_aggressive_optimizations(ir_code);
        }
        
        if (options.print_stages) {
            std::cout << "Optimization: Applied O" << static_cast<int>(options.opt_level) << " optimizations" << std::endl;
        }
        
        if (options.verbose) {
            profiler->end_phase("optimization");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("Optimization failed: " + std::string(e.what()));
        return false;
    }
}

bool CompilerDriver::run_code_generation(const std::string& output_file) {
    if (options.verbose) {
        profiler->start_phase("code_generation");
    }
    
    try {
        // Use stored AST, don't parse again
        if (!ast) {
            error_messages.push_back("Code generation failed: No AST");
            return false;
        }
        
        Program* program = dynamic_cast<Program*>(ast.get());
        auto ir_code = ir_generator->generate(*program);
        
        // Apply optimizations if enabled
        if (options.opt_level >= OptimizationLevel::O1) {
            ir_code = optimizer->optimize(ir_code);
        }
        
        code_gen = std::make_unique<AssemblyGenerator>(output_file);
        code_gen->generate_from_ir(ir_code);
        
        if (options.debug_info) {
            debug_gen->generate_debug_info(output_file + ".debug");
        }
        
        if (options.print_assembly) {
            std::ifstream asm_file(output_file);
            std::string line;
            std::cout << "Generated Assembly:" << std::endl;
            while (std::getline(asm_file, line)) {
                std::cout << line << std::endl;
            }
        }
        
        if (options.print_stages) {
            std::cout << "Code Generation: Assembly generated to " << output_file << std::endl;
        }
        
        if (options.verbose) {
            profiler->end_phase("code_generation");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("Code generation failed: " + std::string(e.what()));
        return false;
    }
}

bool CompilerDriver::run_assembly_and_linking(const std::string& assembly_file, const std::string& output_file) {
    if (options.verbose) {
        profiler->start_phase("assembly_linking");
    }
    
    try {
        // Assemble
        std::string object_file = get_temporary_filename(".o");
        std::string as_cmd = "as -64 " + assembly_file + " -o " + object_file;
        
        if (options.verbose) {
            std::cout << "Assembling: " << as_cmd << std::endl;
        }
        
        int as_result = std::system(as_cmd.c_str());
        if (as_result != 0) {
            error_messages.push_back("Assembly failed");
            return false;
        }
        
        // Link
        std::string ld_cmd = "ld " + object_file + " -o " + output_file;
        
        if (options.verbose) {
            std::cout << "Linking: " << ld_cmd << std::endl;
        }
        
        int ld_result = std::system(ld_cmd.c_str());
        if (ld_result != 0) {
            error_messages.push_back("Linking failed");
            return false;
        }
        
        if (options.print_stages) {
            std::cout << "Assembly & Linking: Executable generated to " << output_file << std::endl;
        }
        
        if (options.verbose) {
            profiler->end_phase("assembly_linking");
        }
        
        return true;
    } catch (const std::exception& e) {
        error_messages.push_back("Assembly/linking failed: " + std::string(e.what()));
        return false;
    }
}

// Option setters and helpers
void CompilerDriver::set_optimization_level(OptimizationLevel level) {
    options.opt_level = level;
}

void CompilerDriver::enable_debug_info(bool enable) {
    options.debug_info = enable;
}

void CompilerDriver::set_verbose(bool verbose) {
    options.verbose = verbose;
}

void CompilerDriver::print_compilation_stages(bool enable) {
    options.print_stages = enable;
}

void CompilerDriver::set_output_format(OutputFormat format) {
    options.output_format = format;
}

void CompilerDriver::enable_profiling(bool enable) {
    if (profiler) {
        profiler->enable_profiling(enable);
    }
}

const std::vector<std::string>& CompilerDriver::get_errors() const {
    return error_messages;
}

const std::vector<std::string>& CompilerDriver::get_warnings() const {
    return warning_messages;
}

void CompilerDriver::clear_messages() {
    error_messages.clear();
    warning_messages.clear();
}

std::string CompilerDriver::get_version() const {
    return "C-- Compiler v1.0.0";
}

std::string CompilerDriver::get_target_info() const {
    return "Target: x86_64-linux-gnu";
}

void CompilerDriver::print_help() const {
    std::cout << "C-- Compiler v1.0\n"
              << "Usage: cmmc [options] <source-file> -o <output-file>\n\n"
              << "Options:\n"
              << "  -O0, -O1, -O2, -O3    Set optimization level\n"
              << "  -g                    Generate debug information\n"
              << "  -v                    Verbose output\n"
              << "  -S                    Generate assembly only\n"
              << "  -c                    Generate object file only\n"
              << "  --print-stages        Print compilation stages\n"
              << "  --print-ir            Print intermediate representation\n"
              << "  --print-asm           Print generated assembly\n"
              << "  --keep-intermediate   Keep intermediate files\n"
              << "  --help                Show this help message\n"
              << "  --version             Show version information\n";
}

void CompilerDriver::print_performance_report() const {
    if (profiler) {
        profiler->generate_performance_report();
    }
}

std::string CompilerDriver::get_temporary_filename(const std::string& suffix) {
    static int counter = 0;
    return "temp_" + std::to_string(counter++) + suffix;
}

void CompilerDriver::cleanup_intermediate_files() {
    // Remove temporary files
    std::filesystem::remove("temp.s");
    std::filesystem::remove("temp.o");
    for (int i = 0; i < 100; ++i) {
        std::filesystem::remove("temp_" + std::to_string(i) + ".s");
        std::filesystem::remove("temp_" + std::to_string(i) + ".o");
    }
}

void CompilerDriver::print_stage_info(const std::string& stage_name, bool success) {
    if (options.print_stages) {
        std::cout << stage_name << ": " << (success ? "✓" : "✗") << std::endl;
    }
}
