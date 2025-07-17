#include "runtime-generator.h"
#include <iostream>

RuntimeGenerator::RuntimeGenerator(std::ofstream& output) : output_file(output) {}

void RuntimeGenerator::generate_startup_code() {
    emit_comment("Program startup code");
    emit_label("_start");
    emit_instruction("call main");
    emit_instruction("mov rdi, rax");        // Exit code from main
    emit_instruction("mov rax, 60");         // sys_exit
    emit_instruction("syscall");
    emit_instruction("");
}

void RuntimeGenerator::generate_io_functions() {
    emit_comment("I/O Functions");
    
    // Input function
    emit_label("input");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("sub rsp, 32");
    
    emit_instruction("mov rax, 0");           // sys_read
    emit_instruction("mov rdi, 0");           // stdin
    emit_instruction("lea rsi, [rbp-32]");    // buffer
    emit_instruction("mov rdx, 32");          // buffer size
    emit_instruction("syscall");
    
    emit_instruction("lea rdi, [rbp-32]");
    emit_instruction("call parse_int");
    
    emit_instruction("mov rsp, rbp");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
    
    // Output function
    emit_label("output");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("sub rsp, 32");
    
    emit_instruction("mov rax, [rbp + 16]");  // Get parameter
    emit_instruction("lea rsi, [rbp-32]");    // buffer
    emit_instruction("mov rdi, rax");
    emit_instruction("call format_int");
    
    emit_instruction("mov rax, 1");           // sys_write
    emit_instruction("mov rdi, 1");           // stdout
    emit_instruction("lea rsi, [rbp-32]");    // buffer
    emit_instruction("mov rdx, rax");         // length returned by format_int
    emit_instruction("syscall");
    
    emit_instruction("mov rsp, rbp");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
}

void RuntimeGenerator::generate_memory_management() {
    emit_comment("Memory Management Functions");
    
    // Simple malloc implementation using brk system call
    emit_label("malloc");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    
    emit_instruction("mov rax, 12");          // sys_brk
    emit_instruction("mov rdi, 0");           // get current break
    emit_instruction("syscall");
    emit_instruction("mov rbx, rax");         // save current break
    
    emit_instruction("add rax, [rbp + 16]");  // add requested size
    emit_instruction("mov rdi, rax");
    emit_instruction("mov rax, 12");          // sys_brk
    emit_instruction("syscall");
    
    emit_instruction("mov rax, rbx");         // return old break (allocated memory)
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
    
    // Free function (stub - real implementation would need heap management)
    emit_label("free");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
}

void RuntimeGenerator::generate_error_handling() {
    emit_comment("Error Handling Functions");
    
    // Runtime error handler
    emit_label("runtime_error");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    
    emit_instruction("mov rax, 1");           // sys_write
    emit_instruction("mov rdi, 2");           // stderr
    emit_instruction("mov rsi, error_msg");
    emit_instruction("mov rdx, error_msg_len");
    emit_instruction("syscall");
    
    emit_instruction("mov rdi, 1");           // exit code
    emit_instruction("mov rax, 60");          // sys_exit
    emit_instruction("syscall");
    emit_instruction("");
    
    // Division by zero handler
    emit_label("div_by_zero");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    
    emit_instruction("mov rax, 1");           // sys_write
    emit_instruction("mov rdi, 2");           // stderr
    emit_instruction("mov rsi, div_zero_msg");
    emit_instruction("mov rdx, div_zero_msg_len");
    emit_instruction("syscall");
    
    emit_instruction("mov rdi, 1");           // exit code
    emit_instruction("mov rax, 60");          // sys_exit
    emit_instruction("syscall");
    emit_instruction("");
}

void RuntimeGenerator::generate_syscall_wrappers() {
    emit_comment("System Call Wrappers");
    
    // Exit wrapper
    emit_label("exit");
    emit_instruction("mov rax, 60");          // sys_exit
    emit_instruction("syscall");
    emit_instruction("");
    
    // Write wrapper
    emit_label("write");
    emit_instruction("mov rax, 1");           // sys_write
    emit_instruction("syscall");
    emit_instruction("ret");
    emit_instruction("");
    
    // Read wrapper
    emit_label("read");
    emit_instruction("mov rax, 0");           // sys_read
    emit_instruction("syscall");
    emit_instruction("ret");
    emit_instruction("");
}

void RuntimeGenerator::generate_string_functions() {
    emit_comment("String Utility Functions");
    
    // Parse integer from string
    emit_label("parse_int");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("xor rax, rax");         // result = 0
    emit_instruction("xor rcx, rcx");         // index = 0
    emit_instruction("mov rbx, 10");          // base = 10
    
    emit_label("parse_int_loop");
    emit_instruction("movzx rdx, byte ptr [rdi + rcx]");
    emit_instruction("cmp rdx, 10");          // newline
    emit_instruction("je parse_int_done");
    emit_instruction("cmp rdx, 0");           // null terminator
    emit_instruction("je parse_int_done");
    emit_instruction("cmp rdx, 32");          // space
    emit_instruction("je parse_int_done");
    emit_instruction("sub rdx, 48");          // convert '0'-'9' to 0-9
    emit_instruction("imul rax, rbx");        // result *= 10
    emit_instruction("add rax, rdx");         // result += digit
    emit_instruction("inc rcx");              // index++
    emit_instruction("jmp parse_int_loop");
    
    emit_label("parse_int_done");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
    
    // Format integer to string
    emit_label("format_int");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("push rbx");
    emit_instruction("mov rax, rdi");         // number to format
    emit_instruction("mov rbx, 10");          // base = 10
    emit_instruction("mov rcx, 0");           // digit count
    
    emit_label("format_int_loop");
    emit_instruction("xor rdx, rdx");
    emit_instruction("div rbx");              // rax = rax / 10, rdx = rax % 10
    emit_instruction("add rdx, 48");          // convert digit to ASCII
    emit_instruction("mov [rsi + rcx], dl");  // store digit
    emit_instruction("inc rcx");              // digit count++
    emit_instruction("test rax, rax");
    emit_instruction("jnz format_int_loop");
    
    // Reverse the string
    emit_instruction("mov rdi, rsi");         // string start
    emit_instruction("lea rsi, [rsi + rcx - 1]"); // string end
    emit_label("reverse_loop");
    emit_instruction("cmp rdi, rsi");
    emit_instruction("jge reverse_done");
    emit_instruction("mov al, [rdi]");
    emit_instruction("mov bl, [rsi]");
    emit_instruction("mov [rdi], bl");
    emit_instruction("mov [rsi], al");
    emit_instruction("inc rdi");
    emit_instruction("dec rsi");
    emit_instruction("jmp reverse_loop");
    
    emit_label("reverse_done");
    emit_instruction("mov byte ptr [rdi + rcx], 10"); // add newline
    emit_instruction("mov rax, rcx");         // return length
    emit_instruction("pop rbx");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
}

void RuntimeGenerator::generate_math_functions() {
    emit_comment("Math Functions");
    
    // Absolute value
    emit_label("abs");
    emit_instruction("cmp rdi, 0");
    emit_instruction("jge abs_positive");
    emit_instruction("neg rdi");
    emit_label("abs_positive");
    emit_instruction("mov rax, rdi");
    emit_instruction("ret");
    emit_instruction("");
    
    // Power function (simple implementation)
    emit_label("power");
    emit_instruction("push rbp");
    emit_instruction("mov rbp, rsp");
    emit_instruction("mov rax, 1");           // result = 1
    emit_instruction("test rsi, rsi");        // check if exponent is 0
    emit_instruction("jz power_done");
    
    emit_label("power_loop");
    emit_instruction("imul rax, rdi");        // result *= base
    emit_instruction("dec rsi");              // exponent--
    emit_instruction("jnz power_loop");
    
    emit_label("power_done");
    emit_instruction("pop rbp");
    emit_instruction("ret");
    emit_instruction("");
}

void RuntimeGenerator::generate_data_section() {
    emit_instruction(".section .data");
    emit_instruction("error_msg: .ascii \"Runtime Error\\n\"");
    emit_instruction("error_msg_len = . - error_msg");
    emit_instruction("div_zero_msg: .ascii \"Division by Zero\\n\"");
    emit_instruction("div_zero_msg_len = . - div_zero_msg");
    emit_instruction("");
    
    emit_instruction(".section .bss");
    emit_instruction("input_buffer: .space 64");
    emit_instruction("output_buffer: .space 64");
    emit_instruction("temp_buffer: .space 64");
    emit_instruction("");
}

// Output helpers
void RuntimeGenerator::emit_instruction(const std::string& instr) {
    output_file << "    " << instr << std::endl;
}

void RuntimeGenerator::emit_label(const std::string& label) {
    output_file << label << ":" << std::endl;
}

void RuntimeGenerator::emit_comment(const std::string& comment) {
    output_file << "    # " << comment << std::endl;
}
