#include "assembly-templates.h"
#include <sstream>
#include <algorithm>

AssemblyTemplates::AssemblyTemplates() {
    initialize_templates();
}

void AssemblyTemplates::initialize_templates() {
    templates["function_call"] = R"(
    push rbp
    mov rbp, rsp
    {PARAMETER_SETUP}
    call {FUNCTION_NAME}
    add rsp, {STACK_CLEANUP}
    {RESULT_HANDLING}
    pop rbp
)";
    
    templates["array_access"] = R"(
    mov {REG1}, {ARRAY_BASE}
    mov {REG2}, {INDEX}
    mov {RESULT}, [{REG1} + {REG2} * 8]
)";
    
    templates["array_assign"] = R"(
    mov {REG1}, {ARRAY_BASE}
    mov {REG2}, {INDEX}
    mov {REG3}, {VALUE}
    mov [{REG1} + {REG2} * 8], {REG3}
)";
    
    templates["conditional_jump"] = R"(
    mov {REG}, {CONDITION}
    test {REG}, {REG}
    {JUMP_INSTRUCTION} {LABEL}
)";
    
    templates["loop"] = R"(
{LOOP_START}:
    {CONDITION_CHECK}
    test {CONDITION_REG}, {CONDITION_REG}
    jz {LOOP_END}
    {LOOP_BODY}
    jmp {LOOP_START}
{LOOP_END}:
)";
    
    templates["syscall"] = R"(
    mov rax, {SYSCALL_NUMBER}
    mov rdi, {ARG1}
    mov rsi, {ARG2}
    mov rdx, {ARG3}
    syscall
)";
    
    templates["register_save"] = R"(
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
)";
    
    templates["register_restore"] = R"(
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
)";
    
    templates["stack_alloc"] = R"(
    sub rsp, {SIZE}
)";
    
    templates["stack_dealloc"] = R"(
    add rsp, {SIZE}
)";
}

std::string AssemblyTemplates::generate_function_call(const std::string& func_name, 
                                                     const std::vector<std::string>& args) {
    std::map<std::string, std::string> params;
    params["FUNCTION_NAME"] = func_name;
    params["STACK_CLEANUP"] = std::to_string(args.size() * 8);
    
    std::ostringstream param_setup;
    for (int i = args.size() - 1; i >= 0; --i) {
        param_setup << "    push " << args[i] << std::endl;
    }
    params["PARAMETER_SETUP"] = param_setup.str();
    
    params["RESULT_HANDLING"] = "    # Result in rax";
    
    return substitute_parameters(templates["function_call"], params);
}

std::string AssemblyTemplates::generate_array_access(const std::string& array, 
                                                     const std::string& index,
                                                     const std::string& result) {
    std::map<std::string, std::string> params;
    params["ARRAY_BASE"] = array;
    params["INDEX"] = index;
    params["RESULT"] = result;
    params["REG1"] = "rbx";
    params["REG2"] = "rcx";
    
    return substitute_parameters(templates["array_access"], params);
}

std::string AssemblyTemplates::generate_conditional_jump(const std::string& condition,
                                                         const std::string& label) {
    std::map<std::string, std::string> params;
    params["CONDITION"] = condition;
    params["LABEL"] = label;
    params["REG"] = "rax";
    params["JUMP_INSTRUCTION"] = "jz";
    
    return substitute_parameters(templates["conditional_jump"], params);
}

std::string AssemblyTemplates::generate_loop_structure(const std::string& condition,
                                                       const std::string& body) {
    std::map<std::string, std::string> params;
    params["LOOP_START"] = "loop_start";
    params["LOOP_END"] = "loop_end";
    params["CONDITION_CHECK"] = condition;
    params["CONDITION_REG"] = "rax";
    params["LOOP_BODY"] = body;
    
    return substitute_parameters(templates["loop"], params);
}

std::string AssemblyTemplates::generate_system_call(const std::string& syscall_name,
                                                    const std::vector<std::string>& args) {
    std::map<std::string, std::string> params;
    
    std::map<std::string, std::string> syscall_numbers = {
        {"read", "0"},
        {"write", "1"},
        {"exit", "60"},
        {"brk", "12"}
    };
    
    params["SYSCALL_NUMBER"] = syscall_numbers[syscall_name];
    params["ARG1"] = args.size() > 0 ? args[0] : "0";
    params["ARG2"] = args.size() > 1 ? args[1] : "0";
    params["ARG3"] = args.size() > 2 ? args[2] : "0";
    
    return substitute_parameters(templates["syscall"], params);
}

std::string AssemblyTemplates::generate_register_save() {
    return templates["register_save"];
}

std::string AssemblyTemplates::generate_register_restore() {
    return templates["register_restore"];
}

std::string AssemblyTemplates::generate_stack_allocation(int size) {
    std::map<std::string, std::string> params;
    params["SIZE"] = std::to_string(size);
    return substitute_parameters(templates["stack_alloc"], params);
}

std::string AssemblyTemplates::generate_stack_deallocation(int size) {
    std::map<std::string, std::string> params;
    params["SIZE"] = std::to_string(size);
    return substitute_parameters(templates["stack_dealloc"], params);
}

std::string AssemblyTemplates::get_template(const std::string& name) const {
    auto it = templates.find(name);
    if (it != templates.end()) {
        return it->second;
    }
    return "";
}

void AssemblyTemplates::add_template(const std::string& name, const std::string& template_str) {
    templates[name] = template_str;
}

std::string AssemblyTemplates::substitute_parameters(const std::string& template_str, 
                                                     const std::map<std::string, std::string>& params) {
    std::string result = template_str;
    
    for (const auto& param : params) {
        std::string placeholder = "{" + param.first + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), param.second);
            pos += param.second.length();
        }
    }
    
    return result;
}
