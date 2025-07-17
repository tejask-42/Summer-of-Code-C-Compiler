#pragma once

#include "ir-types.h"
#include <vector>
#include <set>
#include <memory>
#include <unordered_map>
#include <string>

// Basic block representation
class BasicBlock {
public:
    std::vector<IRInstruction> instructions;
    std::set<BasicBlock*> predecessors;
    std::set<BasicBlock*> successors;
    std::string label;
    int id;
    
    BasicBlock(int block_id) : id(block_id) {}
    
    // Add instruction to block
    void add_instruction(const IRInstruction& instr) {
        instructions.push_back(instr);
    }
    
    // Add predecessor
    void add_predecessor(BasicBlock* pred) {
        predecessors.insert(pred);
    }
    
    // Add successor
    void add_successor(BasicBlock* succ) {
        successors.insert(succ);
    }
    
    // Check if block is empty
    bool is_empty() const {
        return instructions.empty();
    }
    
    // Get first instruction
    const IRInstruction* get_first_instruction() const {
        return instructions.empty() ? nullptr : &instructions[0];
    }
    
    // Get last instruction
    const IRInstruction* get_last_instruction() const {
        return instructions.empty() ? nullptr : &instructions.back();
    }
    
    // Print block for debugging
    void print() const {
        std::cout << "Block " << id << " (Label: " << label << "):" << std::endl;
        for (const auto& instr : instructions) {
            std::cout << "  " << instr.to_string() << std::endl;
        }
        std::cout << "  Predecessors: ";
        for (const auto& pred : predecessors) {
            std::cout << pred->id << " ";
        }
        std::cout << std::endl;
        std::cout << "  Successors: ";
        for (const auto& succ : successors) {
            std::cout << succ->id << " ";
        }
        std::cout << std::endl;
    }
};

// Control Flow Graph
class ControlFlowGraph {
private:
    std::vector<std::unique_ptr<BasicBlock>> blocks;
    BasicBlock* entry_block;
    BasicBlock* exit_block;
    std::unordered_map<std::string, BasicBlock*> label_to_block;
    int next_block_id;
    
    // Helper functions for CFG construction
    void identify_basic_blocks(const IRCode& instructions);
    void build_control_flow_edges();
    void connect_blocks();
    
public:
    ControlFlowGraph();
    ~ControlFlowGraph() = default;
    
    // Build CFG from IR instructions
    void build_from_ir(const IRCode& instructions);
    
    // Get entry block
    BasicBlock* get_entry_block() const { return entry_block; }
    
    // Get exit block
    BasicBlock* get_exit_block() const { return exit_block; }
    
    // Get all blocks
    const std::vector<std::unique_ptr<BasicBlock>>& get_blocks() const { return blocks; }
    
    // Find block by label
    BasicBlock* find_block_by_label(const std::string& label) const;
    
    // Print entire CFG
    void print_graph() const;
    
    // Generate DOT format for visualization
    std::string to_dot() const;
    
    // CFG analysis functions
    std::vector<BasicBlock*> get_topological_order() const;
    std::vector<BasicBlock*> get_reverse_postorder() const;
    bool is_reachable(BasicBlock* from, BasicBlock* to) const;
    
    // Clear CFG
    void clear();
    
private:
    // Helper to create new block
    BasicBlock* create_block();
    
    // Helper to add block
    void add_block(std::unique_ptr<BasicBlock> block);
    
    // Helper for topological sort
    void topological_sort_helper(BasicBlock* block, std::set<BasicBlock*>& visited, 
                                 std::vector<BasicBlock*>& result) const;
};
