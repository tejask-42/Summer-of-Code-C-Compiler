#include "register-allocator.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

RegisterAllocator::RegisterAllocator() : next_spill_offset(-16) {
    initialize_registers();
}

void RegisterAllocator::initialize_registers() {
    // x86-64 general purpose registers (excluding rsp, rbp)
    general_purpose_regs = {
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };
    
    // Caller-saved registers (caller must save before function call)
    caller_saved_regs = {
        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    };
    
    // Callee-saved registers (callee must save if used)
    callee_saved_regs = {
        "rbx", "r12", "r13", "r14", "r15"
    };
    
    // Initialize available registers queue
    // Start with caller-saved registers as they're easier to manage
    for (const auto& reg : caller_saved_regs) {
        if (reg != "rax") { // Reserve rax for return values
            available_registers.push(reg);
        }
    }
    
    // Add some callee-saved registers
    for (const auto& reg : callee_saved_regs) {
        available_registers.push(reg);
    }
}

std::string RegisterAllocator::allocate_register() {
    if (available_registers.empty()) {
        // Need to spill a register
        std::string reg_to_spill = select_register_to_spill();
        spill_register(reg_to_spill);
        return reg_to_spill;
    }
    
    std::string reg = available_registers.front();
    available_registers.pop();
    used_registers.insert(reg);
    
    return reg;
}

std::string RegisterAllocator::allocate_register(const std::string& temp) {
    // Check if already allocated
    if (temp_to_register.find(temp) != temp_to_register.end()) {
        return temp_to_register[temp];
    }
    
    std::string reg = allocate_register();
    temp_to_register[temp] = reg;
    
    return reg;
}

void RegisterAllocator::free_register(const std::string& reg) {
    if (used_registers.find(reg) != used_registers.end()) {
        used_registers.erase(reg);
        available_registers.push(reg);
        
        // Remove from temp mapping
        for (auto it = temp_to_register.begin(); it != temp_to_register.end(); ++it) {
            if (it->second == reg) {
                temp_to_register.erase(it);
                break;
            }
        }
    }
}

void RegisterAllocator::free_register_for_temp(const std::string& temp) {
    auto it = temp_to_register.find(temp);
    if (it != temp_to_register.end()) {
        free_register(it->second);
    }
}

void RegisterAllocator::spill_to_memory(const std::string& temp) {
    auto it = temp_to_register.find(temp);
    if (it != temp_to_register.end()) {
        spilled_variables[temp] = next_spill_offset;
        next_spill_offset -= 8;
        
        // Free the register
        free_register(it->second);
    }
}

std::string RegisterAllocator::load_from_memory(const std::string& temp) {
    auto it = spilled_variables.find(temp);
    if (it != spilled_variables.end()) {
        return "[rbp " + std::to_string(it->second) + "]";
    }
    
    throw std::runtime_error("Variable " + temp + " not found in spilled variables");
}

std::string RegisterAllocator::select_register_to_spill() {
    // Simple strategy: spill the least recently used register
    // For now, just pick the first used register
    if (!used_registers.empty()) {
        return *used_registers.begin();
    }
    
    throw std::runtime_error("No registers available for spilling");
}

void RegisterAllocator::spill_register(const std::string& reg) {
    // Find the temp using this register
    std::string temp_to_spill;
    for (const auto& pair : temp_to_register) {
        if (pair.second == reg) {
            temp_to_spill = pair.first;
            break;
        }
    }
    
    if (!temp_to_spill.empty()) {
        spill_to_memory(temp_to_spill);
    }
}

bool RegisterAllocator::is_allocated(const std::string& temp) const {
    return temp_to_register.find(temp) != temp_to_register.end();
}

std::string RegisterAllocator::get_register(const std::string& temp) const {
    auto it = temp_to_register.find(temp);
    if (it != temp_to_register.end()) {
        return it->second;
    }
    return "";
}

bool RegisterAllocator::has_available_registers() const {
    return !available_registers.empty();
}

void RegisterAllocator::save_caller_saved_registers() {
    // Implementation for saving caller-saved registers before function calls
    // This would emit assembly instructions to save registers
}

void RegisterAllocator::restore_caller_saved_registers() {
    // Implementation for restoring caller-saved registers after function calls
}

void RegisterAllocator::save_callee_saved_registers() {
    // Implementation for saving callee-saved registers at function entry
}

void RegisterAllocator::restore_callee_saved_registers() {
    // Implementation for restoring callee-saved registers at function exit
}

void RegisterAllocator::print_allocation_state() const {
    std::cout << "Register Allocation State:" << std::endl;
    std::cout << "Used registers: ";
    for (const auto& reg : used_registers) {
        std::cout << reg << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Temp to register mapping:" << std::endl;
    for (const auto& pair : temp_to_register) {
        std::cout << "  " << pair.first << " -> " << pair.second << std::endl;
    }
    
    std::cout << "Available registers: " << available_registers.size() << std::endl;
}

void RegisterAllocator::reset() {
    temp_to_register.clear();
    used_registers.clear();
    spilled_variables.clear();
    next_spill_offset = -16;
    
    // Reinitialize available registers
    while (!available_registers.empty()) {
        available_registers.pop();
    }
    initialize_registers();
}
