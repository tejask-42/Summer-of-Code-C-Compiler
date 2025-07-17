#include "debug-info-generator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

DebugInfoGenerator::DebugInfoGenerator() : current_address(0) {}

void DebugInfoGenerator::generate_debug_info(const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open debug info file: " + output_file);
    }
    
    // Write debug header
    file << "# Debug Information for C-- Compiler" << std::endl;
    file << "# Generated at compilation time" << std::endl;
    file << "# Source: " << current_source_file << std::endl;
    file << std::endl;
    
    // Write line number information
    file << "# Line Number Information" << std::endl;
    file << "# Address -> Source Location" << std::endl;
    for (const auto& info : line_info) {
        file << std::hex << info.instruction_address << " -> " 
             << info.source_file << ":" << info.line_number 
             << ":" << info.column_number << std::endl;
    }
    file << std::endl;
    
    // Write variable information
    file << "# Variable Information" << std::endl;
    file << "# Name Type Location Scope" << std::endl;
    for (const auto& info : variable_info) {
        file << info.name << " " << info.type << " " << info.location 
             << " " << info.scope << " [" << std::hex << info.start_address 
             << "-" << info.end_address << "]" << std::endl;
    }
    file << std::endl;
    
    // Write debug symbols
    file << "# Debug Symbols" << std::endl;
    for (const auto& symbol : debug_symbols) {
        file << symbol.variable_name << " " << symbol.type_info 
             << " " << symbol.source_file << ":" << symbol.line_number 
             << ":" << symbol.column_number << " " << symbol.scope_name << std::endl;
    }
    
    file.close();
}

void DebugInfoGenerator::generate_dwarf_info(const std::string& output_file) {
    std::ofstream file(output_file, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open DWARF info file: " + output_file);
    }
    
    write_dwarf_header(file);
    write_dwarf_compilation_unit(file);
    write_dwarf_line_info(file);
    write_dwarf_variable_info(file);
    write_dwarf_footer(file);
    
    file.close();
}

void DebugInfoGenerator::emit_line_number_info(const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open line info file: " + output_file);
    }
    
    file << "# Line Number Information" << std::endl;
    file << "# Format: address file:line:column" << std::endl;
    
    for (const auto& info : line_info) {
        file << "0x" << std::hex << std::setw(8) << std::setfill('0') 
             << info.instruction_address << " " << info.source_file 
             << ":" << std::dec << info.line_number << ":" << info.column_number << std::endl;
    }
    
    file.close();
}

void DebugInfoGenerator::emit_variable_info(const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open variable info file: " + output_file);
    }
    
    file << "# Variable Information" << std::endl;
    file << "# Format: name type location start-end scope" << std::endl;
    
    for (const auto& info : variable_info) {
        file << info.name << " " << info.type << " " << info.location 
             << " 0x" << std::hex << info.start_address << "-0x" << info.end_address 
             << " " << info.scope << std::endl;
    }
    
    file.close();
}

void DebugInfoGenerator::set_source_file(const std::string& filename) {
    current_source_file = filename;
}

void DebugInfoGenerator::set_current_function(const std::string& function_name) {
    current_function = function_name;
}

void DebugInfoGenerator::set_current_address(size_t address) {
    current_address = address;
}

void DebugInfoGenerator::register_variable(const std::string& name, const std::string& type,
                                          int line, int column, const std::string& location) {
    add_debug_symbol(name, type, line, column, current_function);
    add_variable_info(name, type, location, current_address, current_address + 100);
}

void DebugInfoGenerator::register_function(const std::string& name, const std::string& return_type,
                                          int line, int column, const std::vector<std::string>& parameters) {
    std::string func_type = return_type + " " + name + "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) func_type += ", ";
        func_type += parameters[i];
    }
    func_type += ")";
    
    add_debug_symbol(name, func_type, line, column, "global");
}

void DebugInfoGenerator::register_parameter(const std::string& name, const std::string& type,
                                           int line, int column, const std::string& location) {
    DebugInfo info;
    info.source_file = current_source_file;
    info.line_number = line;
    info.column_number = column;
    info.variable_name = name;
    info.type_info = type;
    info.scope_name = current_function;
    info.instruction_address = current_address;
    info.is_parameter = true;
    info.is_local = false;
    
    debug_symbols.push_back(info);
    add_variable_info(name, type, location, current_address, current_address + 100);
}

void DebugInfoGenerator::generate_gdb_symbols(const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open GDB symbols file: " + output_file);
    }
    
    file << "# GDB Symbol Information" << std::endl;
    file << "# Compatible with GDB debugging" << std::endl;
    file << std::endl;
    
    // Write function symbols
    file << "# Function Symbols" << std::endl;
    for (const auto& symbol : debug_symbols) {
        if (symbol.type_info.find("(") != std::string::npos) {
            file << "function " << symbol.variable_name << " " 
                 << std::hex << symbol.instruction_address << std::endl;
        }
    }
    
    file << std::endl;
    
    // Write variable symbols
    file << "# Variable Symbols" << std::endl;
    for (const auto& var : variable_info) {
        file << "variable " << var.name << " " << var.type << " " 
             << var.location << " " << std::hex << var.start_address << std::endl;
    }
    
    file.close();
}

void DebugInfoGenerator::generate_source_map(const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open source map file: " + output_file);
    }
    
    file << "# Source Map for Debugging" << std::endl;
    file << "# Maps assembly addresses to source locations" << std::endl;
    file << std::endl;
    
    // Sort line info by address
    auto sorted_lines = line_info;
    std::sort(sorted_lines.begin(), sorted_lines.end(), 
              [](const LineNumberInfo& a, const LineNumberInfo& b) {
                  return a.instruction_address < b.instruction_address;
              });
    
    for (const auto& info : sorted_lines) {
        file << std::hex << info.instruction_address << " " 
             << info.source_file << " " << std::dec << info.line_number 
             << " " << info.column_number << std::endl;
    }
    
    file.close();
}

void DebugInfoGenerator::generate_debug_json(const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open debug JSON file: " + output_file);
    }
    
    file << "{\n";
    file << "  \"debug_info\": {\n";
    file << "    \"source_file\": \"" << current_source_file << "\",\n";
    file << "    \"line_info\": [\n";
    
    for (size_t i = 0; i < line_info.size(); ++i) {
        const auto& info = line_info[i];
        file << "      {\n";
        file << "        \"address\": \"0x" << std::hex << info.instruction_address << "\",\n";
        file << "        \"file\": \"" << info.source_file << "\",\n";
        file << "        \"line\": " << std::dec << info.line_number << ",\n";
        file << "        \"column\": " << info.column_number << "\n";
        file << "      }";
        if (i < line_info.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "    ],\n";
    file << "    \"variables\": [\n";
    
    for (size_t i = 0; i < variable_info.size(); ++i) {
        const auto& var = variable_info[i];
        file << "      {\n";
        file << "        \"name\": \"" << var.name << "\",\n";
        file << "        \"type\": \"" << var.type << "\",\n";
        file << "        \"location\": \"" << var.location << "\",\n";
        file << "        \"start_address\": \"0x" << std::hex << var.start_address << "\",\n";
        file << "        \"end_address\": \"0x" << var.end_address << "\",\n";
        file << "        \"scope\": \"" << var.scope << "\"\n";
        file << "      }";
        if (i < variable_info.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "    ]\n";
    file << "  }\n";
    file << "}\n";
    
    file.close();
}

void DebugInfoGenerator::clear_debug_info() {
    debug_symbols.clear();
    line_info.clear();
    variable_info.clear();
    current_source_file.clear();
    current_function.clear();
    current_address = 0;
}

void DebugInfoGenerator::print_debug_summary() const {
    std::cout << "=== Debug Information Summary ===" << std::endl;
    std::cout << "Source file: " << current_source_file << std::endl;
    std::cout << "Debug symbols: " << debug_symbols.size() << std::endl;
    std::cout << "Line info entries: " << line_info.size() << std::endl;
    std::cout << "Variable info entries: " << variable_info.size() << std::endl;
    std::cout << "=================================" << std::endl;
}

bool DebugInfoGenerator::has_debug_info() const {
    return !debug_symbols.empty() || !line_info.empty() || !variable_info.empty();
}

// Debug info generation and helpers
// ...existing code...
void DebugInfoGenerator::write_dwarf_header(std::ofstream&) {}
void DebugInfoGenerator::write_dwarf_compilation_unit(std::ofstream&) {}
void DebugInfoGenerator::write_dwarf_line_info(std::ofstream&) {}
void DebugInfoGenerator::write_dwarf_variable_info(std::ofstream&) {}
void DebugInfoGenerator::write_dwarf_footer(std::ofstream&) {}

void DebugInfoGenerator::add_debug_symbol(
    const std::string&, const std::string&, int, int, const std::string&) {}

void DebugInfoGenerator::add_variable_info(
    const std::string&, const std::string&, const std::string&, unsigned long, unsigned long) {}