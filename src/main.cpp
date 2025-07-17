#include <iostream>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "semantic-analyzer.h"
#include "ir-generator.h"
#include "ir-optimizer.h"
#include "cfg.h"
#include "assembly-generator.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file> [--tokens] [--ast] [--symbols] [--ir] [--cfg] [--asm]\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Could not open file: " << argv[1] << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    
    // Parse command line options
    bool show_tokens = false, show_ast = false, show_symbols = false, 
         show_ir = false, show_cfg = false, show_asm = false;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tokens") show_tokens = true;
        else if (arg == "--ast") show_ast = true;
        else if (arg == "--symbols") show_symbols = true;
        else if (arg == "--ir") show_ir = true;
        else if (arg == "--cfg") show_cfg = true;
        else if (arg == "--asm") show_asm = true;
    }
    
    std::cout << "=== C-- Compiler ===" << std::endl;
    std::cout << "Compiling: " << argv[1] << std::endl << std::endl;
    
    try {
        // Phase 1: Lexical Analysis
        std::cout << "Phase 1: Lexical Analysis..." << std::endl;
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        std::cout << "  ✓ Generated " << tokens.size() << " tokens" << std::endl;
        
        if (show_tokens) {
            std::cout << "\n--- Tokens ---" << std::endl;
            for (const auto& token : tokens) {
                std::cout << "  " << TokenTypeToString(token.type(), token.value()) 
                         << " at line " << token.line() << ", col " << token.column() << std::endl;
            }
        }
        
        // Phase 2: Syntax Analysis
        std::cout << "\nPhase 2: Syntax Analysis..." << std::endl;
        Parser parser(tokens);
        auto ast = parser.parse_program();
        
        if (!ast) {
            std::cerr << "  ✗ Parsing failed" << std::endl;
            return 1;
        }
        
        std::cout << "  ✓ AST generated successfully" << std::endl;
        
        if (show_ast) {
            std::cout << "\n--- AST ---" << std::endl;
            ast->print();
        }
        
        // Phase 3: Semantic Analysis
        std::cout << "\nPhase 3: Semantic Analysis..." << std::endl;
        SemanticAnalyzer analyzer;
        
        Program* program = dynamic_cast<Program*>(ast.get());
        if (!program) {
            std::cerr << "  ✗ AST root is not a Program node" << std::endl;
            return 1;
        }
        
        bool semantic_success = analyzer.analyze(*program);
        
        if (!semantic_success) {
            std::cout << "  ✗ Semantic analysis failed" << std::endl;
            std::cout << "\n--- Semantic Errors ---" << std::endl;
            analyzer.get_error_collector().print_errors();
            return 1;
        }
        
        std::cout << "  ✓ Semantic analysis passed" << std::endl;
        
        if (show_symbols) {
            std::cout << "\n--- Symbol Table ---" << std::endl;
            analyzer.print_symbol_table();
        }
        
        // Phase 4: IR Generation
        std::cout << "\nPhase 4: IR Generation..." << std::endl;
        IRGenerator ir_generator(&analyzer);
        auto ir_code = ir_generator.generate(*program);
        std::cout << "  ✓ Generated " << ir_code.size() << " IR instructions" << std::endl;
        
        if (show_ir) {
            std::cout << "\n--- Generated IR ---" << std::endl;
            ir_generator.print_ir();
        }
        
        // Phase 5: IR Optimization
        std::cout << "\nPhase 5: IR Optimization..." << std::endl;
        IROptimizer optimizer;
        auto optimized_ir = optimizer.optimize(ir_code);
        std::cout << "  ✓ Optimized IR (reduced from " << ir_code.size() 
                  << " to " << optimized_ir.size() << " instructions)" << std::endl;
        
        // Phase 6: Control Flow Graph Construction
        std::cout << "\nPhase 6: Control Flow Graph Construction..." << std::endl;
        ControlFlowGraph cfg;
        cfg.build_from_ir(optimized_ir);
        std::cout << "  ✓ Built CFG with " << cfg.get_blocks().size() << " basic blocks" << std::endl;
        
        if (show_cfg) {
            std::cout << "\n--- Control Flow Graph ---" << std::endl;
            cfg.print_graph();
        }
        
        // Phase 7: Assembly Generation
        std::cout << "\nPhase 7: Assembly Generation..." << std::endl;
        std::string asm_filename = std::string(argv[1]) + ".s";
        AssemblyGenerator asm_generator(asm_filename);
        asm_generator.generate_from_ir(optimized_ir);
        std::cout << "  ✓ Generated assembly code: " << asm_filename << std::endl;
        
        if (show_asm) {
            std::cout << "\n--- Generated Assembly ---" << std::endl;
            std::ifstream asm_file(asm_filename);
            std::string line;
            while (std::getline(asm_file, line)) {
                std::cout << line << std::endl;
            }
        }
        
        std::cout << "\n=== Compilation Successful ===" << std::endl;
        std::cout << "Assembly file generated: " << asm_filename << std::endl;
        std::cout << "To assemble and link: as -64 " << asm_filename << " -o program.o && ld program.o -o program" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during compilation: " << e.what() << std::endl;
        return 1;
    }
}
