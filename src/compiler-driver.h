#pragma once

#include "lexer.h"
#include "parser.h"
#include "semantic-analyzer.h"
#include "ir-generator.h"
#include "ir-optimizer.h"
#include "assembly-generator.h"
#include "advanced-optimizer.h"
#include "debug-info-generator.h"
#include "compiler-profiler.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

enum class OptimizationLevel {
    O0 = 0,  // No optimization
    O1 = 1,  // Basic optimization
    O2 = 2,  // Standard optimization
    O3 = 3   // Aggressive optimization
};

enum class OutputFormat {
    ASSEMBLY,
    OBJECT,
    EXECUTABLE
};

struct CompilerOptions {
    OptimizationLevel opt_level = OptimizationLevel::O0;
    bool debug_info = false;
    bool verbose = false;
    bool print_stages = false;
    bool print_ir = false;
    bool print_cfg = false;
    bool print_assembly = false;
    bool keep_intermediate = false;
    OutputFormat output_format = OutputFormat::EXECUTABLE;
    std::string target_architecture = "x86_64";
    std::vector<std::string> include_paths;
    std::vector<std::string> library_paths;
};

class CompilerDriver {
private:
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<SemanticAnalyzer> analyzer;
    std::unique_ptr<IRGenerator> ir_generator;
    std::unique_ptr<IROptimizer> optimizer;
    std::unique_ptr<AdvancedOptimizer> advanced_optimizer;
    std::unique_ptr<AssemblyGenerator> code_gen;
    std::unique_ptr<DebugInfoGenerator> debug_gen;
    std::unique_ptr<CompilerProfiler> profiler;
    
    // Store parsed AST to reuse across phases
    std::unique_ptr<Program> ast;
    
    CompilerOptions options;
    std::vector<std::string> error_messages;
    std::vector<std::string> warning_messages;
    
    // Compilation pipeline methods
    bool run_lexical_analysis(const std::string& source);
    bool run_syntax_analysis();
    bool run_semantic_analysis();
    bool run_ir_generation();
    bool run_optimization();
    bool run_code_generation(const std::string& output_file);
    bool run_assembly_and_linking(const std::string& assembly_file, const std::string& output_file);
    
    // Utility methods
    void print_stage_info(const std::string& stage_name, bool success);
    void cleanup_intermediate_files();
    std::string get_temporary_filename(const std::string& suffix);
    
public:
    CompilerDriver();
    ~CompilerDriver();
    
    // Main compilation interface
    bool compile(const std::string& source_file, const std::string& output_file);
    bool compile_from_source(const std::string& source_code, const std::string& output_file);
    
    // Configuration methods
    void set_optimization_level(OptimizationLevel level);
    void enable_debug_info(bool enable);
    void set_verbose(bool verbose);
    void print_compilation_stages(bool enable);
    void set_output_format(OutputFormat format);
    void add_include_path(const std::string& path);
    void add_library_path(const std::string& path);
    
    // Error and warning handling
    const std::vector<std::string>& get_errors() const;
    const std::vector<std::string>& get_warnings() const;
    void clear_messages();
    
    // Compiler information
    std::string get_version() const;
    std::string get_target_info() const;
    void print_help() const;
    
    // Performance and profiling
    void enable_profiling(bool enable);
    void print_performance_report() const;
    
    // Testing support
    bool run_self_tests();
    bool validate_installation();
};
