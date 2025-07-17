#pragma once

#include "ir-types.h"
#include "register-allocator.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

class AssemblyGenerator {
private:
    std::unique_ptr<RegisterAllocator> register_allocator;
    std::ofstream output_file;
    int stack_offset;
    int label_counter;
    
    // Function-level state
    std::string current_function;
    std::unordered_map<std::string, int> local_variables;
    int current_stack_size;
    
    // Code generation helpers
    void emit_instruction(const std::string& instr);
    void emit_label(const std::string& label);
    void emit_comment(const std::string& comment);
    
    // Assembly generation for specific IR operations
    void generate_arithmetic(const IRInstruction& instr);
    void generate_comparison(const IRInstruction& instr);
    void generate_assignment(const IRInstruction& instr);
    void generate_branch(const IRInstruction& instr);
    void generate_function_call(const IRInstruction& instr);
    void generate_return(const IRInstruction& instr);
    void generate_array_access(const IRInstruction& instr);
    
    // Memory and register management
    std::string get_operand(const std::string& operand);
    std::string get_memory_location(const std::string& var);
    int get_variable_offset(const std::string& var);
    
    // Function management
    void process_function_begin(const IRInstruction& instr);
    void process_function_end(const IRInstruction& instr);
    
public:
    AssemblyGenerator(const std::string& output_filename);
    ~AssemblyGenerator();
    
    // Main generation entry point
    void generate_from_ir(const IRCode& instructions);
    
    // Function prologue and epilogue
    void emit_function_prologue(const std::string& func_name);
    void emit_function_epilogue();
    
    // Program structure
    void emit_program_header();
    void emit_program_footer();
    
    // Runtime support
    void emit_runtime_functions();
    
    // Utility functions
    void close_output();
    bool is_open() const;
};
