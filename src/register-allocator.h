#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <memory>

class RegisterAllocator {
private:
    std::map<std::string, std::string> temp_to_register;
    std::set<std::string> used_registers;
    std::queue<std::string> available_registers;
    void spill_register(const std::string& reg);
    
    // Register categories
    std::vector<std::string> general_purpose_regs;
    std::vector<std::string> caller_saved_regs;
    std::vector<std::string> callee_saved_regs;
    
    // Spilling support
    std::map<std::string, int> spilled_variables;
    int next_spill_offset;
    
    // Helper functions
    void initialize_registers();
    std::string select_register_to_spill();
    
public:
    RegisterAllocator();
    ~RegisterAllocator() = default;
    
    // Main allocation interface
    std::string allocate_register();
    std::string allocate_register(const std::string& temp);
    void free_register(const std::string& reg);
    void free_register_for_temp(const std::string& temp);
    
    // Spilling interface
    void spill_to_memory(const std::string& temp);
    std::string load_from_memory(const std::string& temp);
    
    // Query interface
    bool is_allocated(const std::string& temp) const;
    std::string get_register(const std::string& temp) const;
    bool has_available_registers() const;
    
    // Function call support
    void save_caller_saved_registers();
    void restore_caller_saved_registers();
    void save_callee_saved_registers();
    void restore_callee_saved_registers();
    
    // Debug and utility
    void print_allocation_state() const;
    void reset();
};
