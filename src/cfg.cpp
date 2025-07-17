#include "cfg.h"
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <algorithm>

ControlFlowGraph::ControlFlowGraph() : entry_block(nullptr), exit_block(nullptr), next_block_id(0) {}

void ControlFlowGraph::build_from_ir(const IRCode& instructions) {
    clear();
    
    if (instructions.empty()) return;
    
    identify_basic_blocks(instructions);
    
    build_control_flow_edges();
    
    connect_blocks();
}

void ControlFlowGraph::identify_basic_blocks(const IRCode& instructions) {
    std::set<size_t> block_starts;
    
    block_starts.insert(0);
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instr = instructions[i];
        
        if (instr.is_label()) {
            block_starts.insert(i);
        }
        
        if (instr.is_branch() && i + 1 < instructions.size()) {
            block_starts.insert(i + 1);
        }
        
        if (instr.op == OpCode::FUNCTION_BEGIN || instr.op == OpCode::FUNCTION_END) {
            if (i + 1 < instructions.size()) {
                block_starts.insert(i + 1);
            }
        }
    }
    
    std::vector<size_t> starts(block_starts.begin(), block_starts.end());
    std::sort(starts.begin(), starts.end());
    
    for (size_t i = 0; i < starts.size(); ++i) {
        size_t start = starts[i];
        size_t end = (i + 1 < starts.size()) ? starts[i + 1] : instructions.size();
        
        auto block = std::make_unique<BasicBlock>(next_block_id++);
        
        for (size_t j = start; j < end; ++j) {
            block->add_instruction(instructions[j]);
            
            if (j == start && instructions[j].is_label()) {
                block->label = instructions[j].result;
                label_to_block[block->label] = block.get();
            }
        }
        
        if (i == 0) {
            entry_block = block.get();
        }
        
        blocks.push_back(std::move(block));
    }
    
    if (!blocks.empty() && blocks.back()->get_last_instruction() && 
        blocks.back()->get_last_instruction()->op != OpCode::RETURN) {
        exit_block = create_block();
    }
}

void ControlFlowGraph::build_control_flow_edges() {
    for (size_t i = 0; i < blocks.size(); ++i) {
        BasicBlock* current = blocks[i].get();
        const IRInstruction* last = current->get_last_instruction();
        
        if (!last) continue;
        
        if (last->op == OpCode::GOTO) {
            BasicBlock* target = find_block_by_label(last->result);
            if (target) {
                current->add_successor(target);
                target->add_predecessor(current);
            }
        } else if (last->op == OpCode::IF_FALSE || last->op == OpCode::IF_TRUE) {
            BasicBlock* target = find_block_by_label(last->result);
            if (target) {
                current->add_successor(target);
                target->add_predecessor(current);
            }
            
            if (i + 1 < blocks.size()) {
                BasicBlock* next = blocks[i + 1].get();
                current->add_successor(next);
                next->add_predecessor(current);
            }
        } else if (last->op == OpCode::RETURN) {
            if (exit_block) {
                current->add_successor(exit_block);
                exit_block->add_predecessor(current);
            }
        } else {
            if (i + 1 < blocks.size()) {
                BasicBlock* next = blocks[i + 1].get();
                current->add_successor(next);
                next->add_predecessor(current);
            }
        }
    }
}

void ControlFlowGraph::connect_blocks() {
}

BasicBlock* ControlFlowGraph::find_block_by_label(const std::string& label) const {
    auto it = label_to_block.find(label);
    return (it != label_to_block.end()) ? it->second : nullptr;
}

void ControlFlowGraph::print_graph() const {
    std::cout << "=== Control Flow Graph ===" << std::endl;
    std::cout << "Entry Block: " << (entry_block ? std::to_string(entry_block->id) : "None") << std::endl;
    std::cout << "Exit Block: " << (exit_block ? std::to_string(exit_block->id) : "None") << std::endl;
    std::cout << "Total Blocks: " << blocks.size() << std::endl;
    std::cout << std::endl;
    
    for (const auto& block : blocks) {
        block->print();
        std::cout << std::endl;
    }
}

std::string ControlFlowGraph::to_dot() const {
    std::ostringstream dot;
    dot << "digraph CFG {\n";
    dot << "  node [shape=box];\n";
    
    for (const auto& block : blocks) {
        dot << "  " << block->id << " [label=\"Block " << block->id;
        if (!block->label.empty()) {
            dot << "\\n" << block->label;
        }
        dot << "\"];\n";
        
        for (const auto& succ : block->successors) {
            dot << "  " << block->id << " -> " << succ->id << ";\n";
        }
    }
    
    dot << "}\n";
    return dot.str();
}

std::vector<BasicBlock*> ControlFlowGraph::get_topological_order() const {
    std::vector<BasicBlock*> result;
    std::set<BasicBlock*> visited;
    
    if (entry_block) {
        topological_sort_helper(entry_block, visited, result);
    }
    
    return result;
}

std::vector<BasicBlock*> ControlFlowGraph::get_reverse_postorder() const {
    auto topo_order = get_topological_order();
    std::reverse(topo_order.begin(), topo_order.end());
    return topo_order;
}

bool ControlFlowGraph::is_reachable(BasicBlock* from, BasicBlock* to) const {
    if (!from || !to) return false;
    if (from == to) return true;
    
    std::set<BasicBlock*> visited;
    std::queue<BasicBlock*> queue;
    
    queue.push(from);
    visited.insert(from);
    
    while (!queue.empty()) {
        BasicBlock* current = queue.front();
        queue.pop();
        
        for (BasicBlock* succ : current->successors) {
            if (succ == to) return true;
            
            if (visited.find(succ) == visited.end()) {
                visited.insert(succ);
                queue.push(succ);
            }
        }
    }
    
    return false;
}

void ControlFlowGraph::clear() {
    blocks.clear();
    label_to_block.clear();
    entry_block = nullptr;
    exit_block = nullptr;
    next_block_id = 0;
}

BasicBlock* ControlFlowGraph::create_block() {
    auto block = std::make_unique<BasicBlock>(next_block_id++);
    BasicBlock* ptr = block.get();
    blocks.push_back(std::move(block));
    return ptr;
}

void ControlFlowGraph::add_block(std::unique_ptr<BasicBlock> block) {
    blocks.push_back(std::move(block));
}

void ControlFlowGraph::topological_sort_helper(BasicBlock* block, std::set<BasicBlock*>& visited, 
                                               std::vector<BasicBlock*>& result) const {
    visited.insert(block);
    
    for (BasicBlock* succ : block->successors) {
        if (visited.find(succ) == visited.end()) {
            topological_sort_helper(succ, visited, result);
        }
    }
    
    result.push_back(block);
}
