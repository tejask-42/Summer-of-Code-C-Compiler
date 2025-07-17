#pragma once

#include "ir-types.h"
#include "cfg.h"
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

// Data flow analysis structures
struct ReachingDefinition {
    std::string variable;
    size_t instruction_index;
    std::string definition_point;
};

struct LivenessInfo {
    std::set<std::string> live_in;
    std::set<std::string> live_out;
    std::set<std::string> use;
    std::set<std::string> def;
};

struct AvailableExpression {
    std::string expression;
    std::string operator_type;
    std::string left_operand;
    std::string right_operand;
    size_t instruction_index;
};

class AdvancedOptimizer {
private:
    // Data flow analysis results
    std::map<size_t, std::set<ReachingDefinition>> reaching_definitions;
    std::map<size_t, LivenessInfo> liveness_info;
    std::map<size_t, std::set<AvailableExpression>> available_expressions;
    
    // Control flow graph
    std::unique_ptr<ControlFlowGraph> cfg;
    
    // Helper methods for data flow analysis
    void initialize_dataflow_sets(const IRCode& instructions);
    bool update_reaching_definitions(const IRCode& instructions);
    bool update_liveness_info(const IRCode& instructions);
    bool update_available_expressions(const IRCode& instructions);
    bool should_reorder_for_performance(const IRInstruction& first, const IRInstruction& second);
    bool is_constant(const std::string& str);
    
    // Helper methods for optimizations
    std::set<std::string> get_variables_used(const IRInstruction& instr);
    std::set<std::string> get_variables_defined(const IRInstruction& instr);
    bool is_loop_invariant(const IRInstruction& instr, const std::set<std::string>& loop_vars);
    std::string get_expression_key(const IRInstruction& instr);
    
public:
    AdvancedOptimizer();
    ~AdvancedOptimizer() = default;
    
    // Main optimization interface
    void apply_dataflow_optimizations(IRCode& instructions);
    void apply_aggressive_optimizations(IRCode& instructions);
    
    // Data flow optimizations
    void reaching_definitions_analysis(const IRCode& instructions);
    void live_variable_analysis(const IRCode& instructions);
    void available_expressions_analysis(const IRCode& instructions);
    
    // Control flow optimizations
    void unreachable_code_elimination(IRCode& instructions);
    void loop_invariant_code_motion(IRCode& instructions);
    void strength_reduction(IRCode& instructions);
    void loop_unrolling(IRCode& instructions);
    void tail_call_optimization(IRCode& instructions);
    
    // Advanced register allocation
    void graph_coloring_allocation(IRCode& instructions);
    void linear_scan_allocation(IRCode& instructions);
    
    // Peephole optimizations
    void peephole_optimizations(IRCode& instructions);
    void instruction_scheduling(IRCode& instructions);
    
    // Utility methods
    void print_dataflow_info() const;
    void print_optimization_stats() const;
    std::string get_optimization_summary() const;
};
