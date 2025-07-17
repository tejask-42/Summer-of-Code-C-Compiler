#pragma once

#include <string>
#include <fstream>
#include <vector>

class RuntimeGenerator {
private:
    std::ofstream& output_file;
    
    void emit_instruction(const std::string& instr);
    void emit_label(const std::string& label);
    void emit_comment(const std::string& comment);
    
public:
    RuntimeGenerator(std::ofstream& output);
    ~RuntimeGenerator() = default;
    
    // Main runtime generation functions
    void generate_startup_code();
    void generate_io_functions();
    void generate_memory_management();
    void generate_error_handling();
    
    // System call wrappers
    void generate_syscall_wrappers();
    
    // Utility functions
    void generate_string_functions();
    void generate_math_functions();
    
    // Data section
    void generate_data_section();
};
