#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

struct DebugInfo {
    std::string source_file;
    int line_number;
    int column_number;
    std::string variable_name;
    std::string type_info;
    std::string scope_name;
    size_t instruction_address;
    bool is_parameter;
    bool is_local;
};

struct LineNumberInfo {
    size_t instruction_address;
    std::string source_file;
    int line_number;
    int column_number;
};

struct VariableInfo {
    std::string name;
    std::string type;
    std::string location; // register or memory location
    size_t start_address;
    size_t end_address;
    std::string scope;
};

class DebugInfoGenerator {
private:
    std::vector<DebugInfo> debug_symbols;
    std::vector<LineNumberInfo> line_info;
    std::vector<VariableInfo> variable_info;
    
    std::string current_source_file;
    std::string current_function;
    size_t current_address;
    
    // DWARF generation helpers
    void write_dwarf_header(std::ofstream& file);
    void write_dwarf_compilation_unit(std::ofstream& file);
    void write_dwarf_line_info(std::ofstream& file);
    void write_dwarf_variable_info(std::ofstream& file);
    void write_dwarf_footer(std::ofstream& file);
    
    // Debug symbol management
    void add_debug_symbol(const std::string& name, const std::string& type, 
                         int line, int column, const std::string& scope);
    void add_line_info(size_t address, int line, int column);
    void add_variable_info(const std::string& name, const std::string& type,
                          const std::string& location, size_t start, size_t end);
    
public:
    DebugInfoGenerator();
    ~DebugInfoGenerator() = default;
    
    // Main debug info generation
    void generate_debug_info(const std::string& output_file);
    void generate_dwarf_info(const std::string& output_file);
    void emit_line_number_info(const std::string& output_file);
    void emit_variable_info(const std::string& output_file);
    
    // Source mapping
    void set_source_file(const std::string& filename);
    void set_current_function(const std::string& function_name);
    void set_current_address(size_t address);
    
    // Symbol registration
    void register_variable(const std::string& name, const std::string& type,
                          int line, int column, const std::string& location);
    void register_function(const std::string& name, const std::string& return_type,
                          int line, int column, const std::vector<std::string>& parameters);
    void register_parameter(const std::string& name, const std::string& type,
                           int line, int column, const std::string& location);
    
    // Debug output formats
    void generate_gdb_symbols(const std::string& output_file);
    void generate_source_map(const std::string& output_file);
    void generate_debug_json(const std::string& output_file);
    
    // Utility methods
    void clear_debug_info();
    void print_debug_summary() const;
    bool has_debug_info() const;
    
    // Integration with compiler phases
    void process_ast_node(const std::string& node_type, int line, int column);
    void process_ir_instruction(const std::string& ir_instr, size_t address);
    void process_assembly_instruction(const std::string& asm_instr, size_t address);
};
