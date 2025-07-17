#include "advanced-optimizer.h"
#include <iostream>
#include <algorithm>
#include <queue>

AdvancedOptimizer::AdvancedOptimizer() {
    cfg = std::make_unique<ControlFlowGraph>();
}

void AdvancedOptimizer::apply_dataflow_optimizations(IRCode& instructions) {
    // Build control flow graph
    cfg->build_from_ir(instructions);
    
    // Perform data flow analyses
    reaching_definitions_analysis(instructions);
    live_variable_analysis(instructions);
    available_expressions_analysis(instructions);
    
    // Apply optimizations based on analysis
    unreachable_code_elimination(instructions);
    
    // Remove dead code based on liveness analysis
    auto it = instructions.begin();
    while (it != instructions.end()) {
        size_t index = std::distance(instructions.begin(), it);
        
        if (it->modifies_result()) {
            std::string defined_var = it->get_defined_variable();
            if (liveness_info.find(index) != liveness_info.end() &&
                liveness_info[index].live_out.find(defined_var) == liveness_info[index].live_out.end()) {
                // Variable is not live after this instruction, remove it
                it = instructions.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void AdvancedOptimizer::apply_aggressive_optimizations(IRCode& instructions) {
    // Apply more aggressive optimizations
    loop_invariant_code_motion(instructions);
    strength_reduction(instructions);
    loop_unrolling(instructions);
    tail_call_optimization(instructions);
    peephole_optimizations(instructions);
    instruction_scheduling(instructions);
}

// Dataflow, loop, and peephole optimizations
void AdvancedOptimizer::reaching_definitions_analysis(const IRCode& instructions) {
    reaching_definitions.clear();
    initialize_dataflow_sets(instructions);
    
    // Iterative data flow analysis
    bool changed = true;
    while (changed) {
        changed = update_reaching_definitions(instructions);
    }
}

void AdvancedOptimizer::live_variable_analysis(const IRCode& instructions) {
    liveness_info.clear();
    
    // Initialize liveness info for each instruction
    for (size_t i = 0; i < instructions.size(); ++i) {
        LivenessInfo info;
        info.use = get_variables_used(instructions[i]);
        info.def = get_variables_defined(instructions[i]);
        liveness_info[i] = info;
    }
    
    // Backward data flow analysis
    bool changed = true;
    while (changed) {
        changed = update_liveness_info(instructions);
    }
}

void AdvancedOptimizer::available_expressions_analysis(const IRCode& instructions) {
    available_expressions.clear();
    
    // Forward data flow analysis
    bool changed = true;
    while (changed) {
        changed = update_available_expressions(instructions);
    }
}

void AdvancedOptimizer::unreachable_code_elimination(IRCode& instructions) {
    std::set<size_t> reachable_instructions;
    std::queue<size_t> worklist;
    
    // Start from entry point
    worklist.push(0);
    reachable_instructions.insert(0);
    
    while (!worklist.empty()) {
        size_t current = worklist.front();
        worklist.pop();
        
        if (current >= instructions.size()) continue;
        
        const auto& instr = instructions[current];
        
        // Add successors to worklist
        if (instr.is_branch()) {
            // Handle branch targets
            for (size_t i = 0; i < instructions.size(); ++i) {
                if (instructions[i].is_label() && instructions[i].result == instr.result) {
                    if (reachable_instructions.find(i) == reachable_instructions.end()) {
                        reachable_instructions.insert(i);
                        worklist.push(i);
                    }
                }
            }
        }
        
        // Add fall-through successor
        if (current + 1 < instructions.size() && 
            instr.op != OpCode::GOTO && instr.op != OpCode::RETURN) {
            if (reachable_instructions.find(current + 1) == reachable_instructions.end()) {
                reachable_instructions.insert(current + 1);
                worklist.push(current + 1);
            }
        }
    }
    
    // Remove unreachable instructions
    auto it = instructions.begin();
    size_t index = 0;
    while (it != instructions.end()) {
        if (reachable_instructions.find(index) == reachable_instructions.end()) {
            it = instructions.erase(it);
        } else {
            ++it;
        }
        ++index;
    }
}

void AdvancedOptimizer::loop_invariant_code_motion(IRCode& instructions) {
    // Simple loop detection and invariant code motion
    std::map<std::string, std::pair<size_t, size_t>> loops; // label -> (start, end)
    
    // Find loops
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (instructions[i].is_branch()) {
            // Look for backward branches
            std::string target = instructions[i].result;
            for (size_t j = 0; j < i; ++j) {
                if (instructions[j].is_label() && instructions[j].result == target) {
                    loops[target] = {j, i};
                    break;
                }
            }
        }
    }
    
    // For each loop, find invariant code
    for (const auto& loop : loops) {
        size_t start = loop.second.first;
        size_t end = loop.second.second;
        
        std::set<std::string> loop_vars;
        
        // Collect variables defined in the loop
        for (size_t i = start; i <= end; ++i) {
            if (instructions[i].modifies_result()) {
                loop_vars.insert(instructions[i].get_defined_variable());
            }
        }
        
        // Find invariant instructions
        std::vector<size_t> invariant_instructions;
        for (size_t i = start + 1; i < end; ++i) {
            if (is_loop_invariant(instructions[i], loop_vars)) {
                invariant_instructions.push_back(i);
            }
        }
        
        // Move invariant instructions before the loop
        for (auto it = invariant_instructions.rbegin(); it != invariant_instructions.rend(); ++it) {
            IRInstruction instr = instructions[*it];
            instructions.erase(instructions.begin() + *it);
            instructions.insert(instructions.begin() + start, instr);
        }
    }
}

void AdvancedOptimizer::strength_reduction(IRCode& instructions) {
    // Replace expensive operations with cheaper ones
    for (auto& instr : instructions) {
        if (instr.op == OpCode::MUL) {
            // Replace multiplication by power of 2 with shift
            if (instr.arg2 == "2") {
                instr.op = OpCode::ADD;
                instr.arg2 = instr.arg1;
            } else if (instr.arg2 == "4") {
                // Could implement left shift by 2, but keep simple for now
                // This is a placeholder for more sophisticated strength reduction
            }
        } else if (instr.op == OpCode::DIV) {
            // Replace division by power of 2 with shift
            if (instr.arg2 == "2") {
                // This would need right shift implementation
                // Placeholder for now
            }
        }
    }
}

void AdvancedOptimizer::loop_unrolling(IRCode& instructions) {
    // Simple loop unrolling for small loops
    // This is a simplified implementation
    
    std::map<std::string, std::pair<size_t, size_t>> loops;
    
    // Find loops
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (instructions[i].is_branch()) {
            std::string target = instructions[i].result;
            for (size_t j = 0; j < i; ++j) {
                if (instructions[j].is_label() && instructions[j].result == target) {
                    loops[target] = {j, i};
                    break;
                }
            }
        }
    }
    
    // Unroll small loops
    for (const auto& loop : loops) {
        size_t start = loop.second.first;
        size_t end = loop.second.second;
        
        if (end - start <= 5) { // Only unroll very small loops
            // Duplicate loop body
            std::vector<IRInstruction> loop_body;
            for (size_t i = start + 1; i < end; ++i) {
                loop_body.push_back(instructions[i]);
            }
            
            // Insert duplicated body before the loop
            instructions.insert(instructions.begin() + end, loop_body.begin(), loop_body.end());
        }
    }
}

void AdvancedOptimizer::tail_call_optimization(IRCode& instructions) {
    // Convert tail calls to jumps
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (instructions[i].op == OpCode::CALL && 
            i + 1 < instructions.size() && 
            instructions[i + 1].op == OpCode::RETURN) {
            
            // Replace call + return with jump
            instructions[i].op = OpCode::GOTO;
            instructions[i].result = instructions[i].arg1;
            instructions[i].arg1 = "";
            instructions[i].arg2 = "";
            
            // Remove the return instruction
            instructions.erase(instructions.begin() + i + 1);
        }
    }
}

void AdvancedOptimizer::peephole_optimizations(IRCode& instructions) {
    // Local optimizations on small instruction windows
    for (size_t i = 0; i < instructions.size() - 1; ++i) {
        // Pattern: load then store of same variable
        if (instructions[i].op == OpCode::ASSIGN && 
            instructions[i + 1].op == OpCode::ASSIGN &&
            instructions[i].result == instructions[i + 1].arg1) {
            
            // Replace with direct assignment
            instructions[i].arg1 = instructions[i + 1].arg1;
            instructions.erase(instructions.begin() + i + 1);
        }
        
        // Pattern: add 0 or multiply by 1
        if (instructions[i].op == OpCode::ADD && instructions[i].arg2 == "0") {
            instructions[i].op = OpCode::ASSIGN;
            instructions[i].arg2 = "";
        }
        
        if (instructions[i].op == OpCode::MUL && instructions[i].arg2 == "1") {
            instructions[i].op = OpCode::ASSIGN;
            instructions[i].arg2 = "";
        }
    }
}

void AdvancedOptimizer::instruction_scheduling(IRCode& instructions) {
    // Simple instruction scheduling to reduce pipeline stalls
    // This is a placeholder for more sophisticated scheduling
    
    // Look for independent instructions that can be reordered
    for (size_t i = 0; i < instructions.size() - 1; ++i) {
        if (!instructions[i].is_branch() && !instructions[i + 1].is_branch()) {
            auto used_i = get_variables_used(instructions[i]);
            auto def_i = get_variables_defined(instructions[i]);
            auto used_i1 = get_variables_used(instructions[i + 1]);
            auto def_i1 = get_variables_defined(instructions[i + 1]);
            
            // Check if instructions can be reordered
            bool can_reorder = true;
            for (const auto& var : def_i) {
                if (used_i1.find(var) != used_i1.end()) {
                    can_reorder = false;
                    break;
                }
            }
            
            if (can_reorder) {
                for (const auto& var : def_i1) {
                    if (used_i.find(var) != used_i.end()) {
                        can_reorder = false;
                        break;
                    }
                }
            }
            
            // If we can reorder and it's beneficial, swap them
            if (can_reorder && should_reorder_for_performance(instructions[i], instructions[i + 1])) {
                std::swap(instructions[i], instructions[i + 1]);
            }
        }
    }
}

bool AdvancedOptimizer::should_reorder_for_performance(const IRInstruction& first, const IRInstruction& second) {
    // Heuristic: put memory operations before arithmetic operations
    if (first.op == OpCode::ASSIGN && 
        (second.op == OpCode::ADD || second.op == OpCode::SUB || 
         second.op == OpCode::MUL || second.op == OpCode::DIV)) {
        return false; // Keep load before arithmetic
    }
    
    return true;
}

// Helper method implementations
std::set<std::string> AdvancedOptimizer::get_variables_used(const IRInstruction& instr) {
    std::set<std::string> used;
    
    if (!instr.arg1.empty() && !is_constant(instr.arg1)) {
        used.insert(instr.arg1);
    }
    
    if (!instr.arg2.empty() && !is_constant(instr.arg2)) {
        used.insert(instr.arg2);
    }
    
    return used;
}

std::set<std::string> AdvancedOptimizer::get_variables_defined(const IRInstruction& instr) {
    std::set<std::string> defined;
    
    if (instr.modifies_result() && !instr.result.empty()) {
        defined.insert(instr.result);
    }
    
    return defined;
}

bool AdvancedOptimizer::is_loop_invariant(const IRInstruction& instr, const std::set<std::string>& loop_vars) {
    // Check if instruction is loop invariant
    auto used_vars = get_variables_used(instr);
    
    for (const auto& var : used_vars) {
        if (loop_vars.find(var) != loop_vars.end()) {
            return false; // Uses loop variable
        }
    }
    
    return true;
}

bool AdvancedOptimizer::is_constant(const std::string& str) {
    if (str.empty()) return false;
    return std::isdigit(str[0]) || (str[0] == '-' && str.length() > 1);
}

void AdvancedOptimizer::print_optimization_stats() const {
    std::cout << "=== Advanced Optimization Statistics ===" << std::endl;
    std::cout << "Reaching definitions computed: " << reaching_definitions.size() << " points" << std::endl;
    std::cout << "Liveness analysis completed: " << liveness_info.size() << " instructions" << std::endl;
    std::cout << "Available expressions: " << available_expressions.size() << " points" << std::endl;
    std::cout << "========================================" << std::endl;
}

void AdvancedOptimizer::initialize_dataflow_sets(const IRCode& instructions) {
    // TODO: implement reaching definition set initialization
    (void)instructions; // Prevent unused parameter warning
}

bool AdvancedOptimizer::update_reaching_definitions(const IRCode& instructions) {
    // TODO: implement reaching defs transfer function
    return false;
}

bool AdvancedOptimizer::update_liveness_info(const IRCode& instructions) {
    // TODO: implement liveness analysis iteration
    return false;
}

bool AdvancedOptimizer::update_available_expressions(const IRCode& instructions) {
    // TODO: implement available expr analysis
    return false;
}

