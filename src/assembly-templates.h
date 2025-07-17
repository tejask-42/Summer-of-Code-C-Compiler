#pragma once

#include <string>
#include <vector>
#include <map>

class AssemblyTemplates {
private:
    std::map<std::string, std::string> templates;
    
    void initialize_templates();
    std::string substitute_parameters(const std::string& template_str, 
                                     const std::map<std::string, std::string>& params);
    
public:
    AssemblyTemplates();
    
    // Template generation methods
    std::string generate_function_call(const std::string& func_name, 
                                      const std::vector<std::string>& args);
    std::string generate_array_access(const std::string& array, 
                                     const std::string& index,
                                     const std::string& result);
    std::string generate_conditional_jump(const std::string& condition,
                                         const std::string& label);
    std::string generate_loop_structure(const std::string& condition,
                                       const std::string& body);
    std::string generate_system_call(const std::string& syscall_name,
                                    const std::vector<std::string>& args);
    
    // Utility templates
    std::string generate_register_save();
    std::string generate_register_restore();
    std::string generate_stack_allocation(int size);
    std::string generate_stack_deallocation(int size);
    
    // Get template by name
    std::string get_template(const std::string& name) const;
    
    // Add custom template
    void add_template(const std::string& name, const std::string& template_str);
};
