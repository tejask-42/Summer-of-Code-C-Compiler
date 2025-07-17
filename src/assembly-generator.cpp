// Assembly Generator: emits x86_64 assembly from IR
#include "assembly-generator.h"
#include <iostream>
#include <sstream>

AssemblyGenerator::AssemblyGenerator(const std::string& output_filename) 
    : register_allocator(std::make_unique<RegisterAllocator>()),
      stack_offset(0), label_counter(0), current_stack_size(0) {
    
    output_file.open(output_filename);
    if (!output_file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_filename);
    }
}

AssemblyGenerator::~AssemblyGenerator() {
    close_output();
}

void AssemblyGenerator::generate_from_ir(const IRCode& instructions) {
    emit_program_header();
    emit_runtime_functions();
    
    for (const auto& instr : instructions) {
        emit_comment("IR: " + instr.to_string());
        
        switch (instr.op) {
            case OpCode::ADD:
            case OpCode::SUB:
            case OpCode::MUL:
            case OpCode::DIV:
            case OpCode::MOD:
                generate_arithmetic(instr);
                break;
                
            case OpCode::EQ:
            case OpCode::NE:
            case OpCode::LT:
            case OpCode::LE:
            case OpCode::GT:
            case OpCode::GE:
                generate_comparison(instr);
                break;
                
            case OpCode::ASSIGN:
            case OpCode::COPY:
                generate_assignment(instr);
                break;
                
            case OpCode::GOTO:
            case OpCode::IF_FALSE:
            case OpCode::IF_TRUE:
                generate_branch(instr);
                break;
                
            case OpCode::CALL:
                generate_function_call(instr);
                break;
                
            case OpCode::RETURN:
                generate_return(instr);
                break;
                
            case OpCode::ARRAY_ACCESS:
            case OpCode::ARRAY_ASSIGN:
                generate_array_access(instr);
                break;
                
            case OpCode::LABEL:
                emit_label(instr.result);
                break;
                
            case OpCode::FUNCTION_BEGIN:
                process_function_begin(instr);
                break;
                
            case OpCode::FUNCTION_END:
                process_function_end(instr);
                break;
                
            case OpCode::PARAM:
                // Handle parameter passing
                {
                    std::string reg = register_allocator->allocate_register();
                    std::string operand = get_operand(instr.arg1);
                    emit_instruction("mov " + reg + ", " + operand);
                    emit_instruction("push " + reg);
                    register_allocator->free_register(reg);
                }
                break;
                
            default:
                emit_comment("Unhandled IR instruction: " + instr.to_string());
                break;
        }
    }
    
    emit_program_footer();
}

void AssemblyGenerator::generate_arithmetic(const IRInstruction& instr) {
    std::string reg1 = register_allocator->allocate_register();
    std::string reg2 = register_allocator->allocate_register();
    
    // Load operands
    emit_instruction("mov " + reg1 + ", " + get_operand(instr.arg1));
    emit_instruction("mov " + reg2 + ", " + get_operand(instr.arg2));
    
    // Perform operation
    switch (instr.op) {
        case OpCode::ADD:
            emit_instruction("add " + reg1 + ", " + reg2);
            break;
        case OpCode::SUB:
            emit_instruction("sub " + reg1 + ", " + reg2);
            break;
        case OpCode::MUL:
            emit_instruction("imul " + reg1 + ", " + reg2);
            break;
        case OpCode::DIV:
            emit_instruction("xor rdx, rdx");  // Clear rdx for division
            emit_instruction("mov rax, " + reg1);
            emit_instruction("idiv " + reg2);
            emit_instruction("mov " + reg1 + ", rax");
            break;
        case OpCode::MOD:
            emit_instruction("xor rdx, rdx");
            emit_instruction("mov rax, " + reg1);
            emit_instruction("idiv " + reg2);
            emit_instruction("mov " + reg1 + ", rdx");
            break;
        default:
            break;
    }
    
    // Store result
    emit_instruction("mov " + get_memory_location(instr.result) + ", " + reg1);
    
    register_allocator->free_register(reg1);
    register_allocator->free_register(reg2);
}

void AssemblyGenerator::generate_comparison(const IRInstruction& instr) {
    std::string reg1 = register_allocator->allocate_register();
    std::string reg2 = register_allocator->allocate_register();
    
    emit_instruction("mov " + reg1 + ", " + get_operand(instr.arg1));
    emit_instruction("mov " + reg2 + ", " + get_operand(instr.arg2));
    emit_instruction("cmp " + reg1 + ", " + reg2);
    
    std::string condition;
    switch (instr.op) {
        case OpCode::EQ: condition = "sete"; break;
        case OpCode::NE: condition = "setne"; break;
        case OpCode::LT: condition = "setl"; break;
        case OpCode::LE: condition = "setle"; break;
        case OpCode::GT: condition = "setg"; break;
        case OpCode::GE: condition = "setge"; break;
        default: condition = "sete"; break;
    }
    
    emit_instruction(condition + " al");
    emit_instruction("movzx " + reg1 + ", al");
    emit_instruction("mov " + get_memory_location(instr.result) + ", " + reg1);
    
    register_allocator->free_register(reg1);
    register_allocator->free_register(reg2);
}

void AssemblyGenerator::generate_assignment(const IRInstruction& instr) {
    std::string reg = register_allocator->allocate_register();
    emit_instruction("mov " + reg + ", " + get_operand(instr.arg1));
    emit_instruction("mov " + get_memory_location(instr.result) + ", " + reg);
    register_allocator->free_register(reg);
}

void AssemblyGenerator::generate_branch(const IRInstruction& instr) {
    if (instr.op == OpCode::GOTO) {
        emit_instruction("jmp " + instr.result);
    } else {
        std::string reg = register_allocator->allocate_register();
        emit_instruction("mov " + reg + ", " + get_operand(instr.arg1));
        emit_instruction("test " + reg + ", " + reg);
        
        if (instr.op == OpCode::IF_FALSE) {
            emit_instruction("jz " + instr.result);
        } else { // IF_TRUE
            emit_instruction("jnz " + instr.result);
        }
        
        register_allocator->free_register(reg);
    }
}

void AssemblyGenerator::generate_function_call(const IRInstruction& instr) {
    // Function call with parameter count in arg2
    emit_instruction("call " + instr.arg1);
    
    // Adjust stack pointer for parameters
    int param_count = std::stoi(instr.arg2);
    if (param_count > 0) {
        emit_instruction("add rsp, " + std::to_string(param_count * 8));
    }
    
    // Store return value
    if (!instr.result.empty()) {
        emit_instruction("mov " + get_memory_location(instr.result) + ", rax");
    }
}

void AssemblyGenerator::generate_return(const IRInstruction& instr) {
    if (!instr.arg1.empty()) {
        std::string reg = register_allocator->allocate_register();
        emit_instruction("mov " + reg + ", " + get_operand(instr.arg1));
        emit_instruction("mov rax, " + reg);
        register_allocator->free_register(reg);
    }
    
    emit_function_epilogue();
    emit_instruction("ret");
}

void AssemblyGenerator::generate_array_access(const IRInstruction& instr) {
    std::string base_reg = register_allocator->allocate_register();
    std::string index_reg = register_allocator->allocate_register();
    
    if (instr.op == OpCode::ARRAY_ACCESS) {
        // result = array[index]
        emit_instruction("mov " + base_reg + ", " + get_memory_location(instr.arg1));
        emit_instruction("mov " + index_reg + ", " + get_operand(instr.arg2));
        emit_instruction("mov " + base_reg + ", [" + base_reg + " + " + index_reg + " * 8]");
        emit_instruction("mov " + get_memory_location(instr.result) + ", " + base_reg);
    } else {
        // array[index] = value
        emit_instruction("mov " + base_reg + ", " + get_memory_location(instr.result));
        emit_instruction("mov " + index_reg + ", " + get_operand(instr.arg1));
        std::string value_reg = register_allocator->allocate_register();
        emit_instruction("mov " + value_reg + ", " + get_operand(instr.arg2));
        emit_instruction("mov [" + base_reg + " + " + index_reg + " * 8], " + value_reg);
        register_allocator->free_register(value_reg);
    }
    
    register_allocator->free_register(base_reg);
    register_allocator->free_register(index_reg);
}

void AssemblyGenerator::process_function_begin(const IRInstruction& instr) {
    current_function = instr.result;
    emit_function_prologue(current_function);
}

void AssemblyGenerator::process_function_end(const IRInstruction& instr) {
    emit_function_epilogue();
    current_function.clear();
}

void AssemblyGenerator::emit_function_prologue(const std::string& func_name) {
    emit_label(func_name);
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    
    // Reserve space for local variables (adjust as needed)
    current_stack_size = 64; // Reserve 64 bytes for local variables
    emit_instruction("sub rsp, " + std::to_string(current_stack_size));
}

void AssemblyGenerator::emit_function_epilogue() {
    emit_instruction("mov rsp, rbp");
    emit_instruction("pop rbp");
}

void AssemblyGenerator::emit_program_header() {
    emit_instruction(".intel_syntax noprefix");
    emit_instruction(".global _start");
    emit_instruction("");
    emit_instruction(".section .text");
    emit_instruction("");
    
    // Program entry point
    emit_label("_start");
    emit_instruction("call main");
    emit_instruction("mov rdi, rax");        // Exit code
    emit_instruction("mov rax, 60");         // sys_exit
    emit_instruction("syscall");
    emit_instruction("");
}

void AssemblyGenerator::emit_program_footer() {
    emit_instruction("");
    emit_instruction(".section .data");
    emit_instruction("input_buffer: .space 16");
    emit_instruction("output_buffer: .space 16");
}

void AssemblyGenerator::emit_runtime_functions() {
    // Input function implementation
    emit_label("input");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("sub rsp, 16");
    
    emit_instruction("mov rax, 0");           // sys_read
    emit_instruction("mov rdi, 0");           // stdin
    emit_instruction("mov rsi, input_buffer");
    emit_instruction("mov rdx, 16");
    emit_instruction("syscall");
    
    emit_instruction("mov rdi, input_buffer");
    emit_instruction("call atoi");            // Convert string to integer
    
    emit_instruction("mov rsp, rbp");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
    
    // Output function implementation
    emit_label("output");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("sub rsp, 16");
    
    emit_instruction("mov rax, [rbp + 16]");  // Get parameter
    emit_instruction("mov rdi, rax");
    emit_instruction("mov rsi, output_buffer");
    emit_instruction("call itoa");            // Convert integer to string
    
    emit_instruction("mov rax, 1");           // sys_write
    emit_instruction("mov rdi, 1");           // stdout
    emit_instruction("mov rsi, output_buffer");
    emit_instruction("mov rdx, 16");
    emit_instruction("syscall");
    
    emit_instruction("mov rsp, rbp");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
    
    // Simple atoi implementation
    emit_label("atoi");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("xor rax, rax");
    emit_instruction("xor rcx, rcx");
    
    emit_label("atoi_loop");
    emit_instruction("movzx rdx, byte ptr [rdi + rcx]");
    emit_instruction("cmp rdx, 10");          // newline
    emit_instruction("je atoi_done");
    emit_instruction("cmp rdx, 0");           // null terminator
    emit_instruction("je atoi_done");
    emit_instruction("sub rdx, 48");          // '0' to 0
    emit_instruction("imul rax, 10");
    emit_instruction("add rax, rdx");
    emit_instruction("inc rcx");
    emit_instruction("jmp atoi_loop");
    
    emit_label("atoi_done");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
    
    // Simple itoa implementation
    emit_label("itoa");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("push rbx");
    emit_instruction("mov rax, rdi");
    emit_instruction("mov rbx, 10");
    emit_instruction("mov rcx, 0");
    
    emit_label("itoa_loop");
    emit_instruction("xor rdx, rdx");
    emit_instruction("div rbx");
    emit_instruction("add rdx, 48");
    emit_instruction("mov [rsi + rcx], dl");
    emit_instruction("inc rcx");
    emit_instruction("test rax, rax");
    emit_instruction("jnz itoa_loop");
    
    emit_instruction("mov byte ptr [rsi + rcx], 10");  // newline
    emit_instruction("pop rbx");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
}

// Output helpers
std::string AssemblyGenerator::get_operand(const std::string& operand) {
    if (operand.empty()) return "";
    
    // Check if it's a constant
    if (std::isdigit(operand[0]) || (operand[0] == '-' && operand.length() > 1)) {
        return operand;
    }
    
    // It's a variable or temporary
    return get_memory_location(operand);
}

std::string AssemblyGenerator::get_memory_location(const std::string& var) {
    // Simple stack-based allocation
    static std::unordered_map<std::string, int> var_offsets;
    static int next_offset = -8;
    
    if (var_offsets.find(var) == var_offsets.end()) {
        var_offsets[var] = next_offset;
        next_offset -= 8;
    }
    
    return "[rbp " + std::to_string(var_offsets[var]) + "]";
}

void AssemblyGenerator::emit_instruction(const std::string& instr) {
    output_file << "    " << instr << std::endl;
}

void AssemblyGenerator::emit_label(const std::string& label) {
    output_file << label << ":" << std::endl;
}

void AssemblyGenerator::emit_comment(const std::string& comment) {
    output_file << "    # " << comment << std::endl;
}

void AssemblyGenerator::close_output() {
    if (output_file.is_open()) {
        output_file.close();
    }
}

bool AssemblyGenerator::is_open() const {
    return output_file.is_open();
}
