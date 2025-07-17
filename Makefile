CXX = g++

# Add pthread for gtest and other thread-safe features
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src -pthread
LDFLAGS_TEST = -pthread -lgtest -lgtest_main

# Directories
SRCDIR = src
TESTDIR = tests
OBJDIR = obj
BINDIR = bin

# Create directories if not exist
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Exclude cmmc.cpp for main executable objects (remove main.o if present)
MAIN_SOURCES = $(filter-out $(SRCDIR)/cmmc.cpp,$(SOURCES))
MAIN_OBJECTS = $(MAIN_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Test source files
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.cpp=$(OBJDIR)/%.o)

# Specific test object files for separate test executables
OBJ_ASSEMBLY_TESTS = $(OBJDIR)/assembly_tests.o
OBJ_IR_TESTS       = $(OBJDIR)/ir_tests.o
OBJ_PARSER_TESTS   = $(OBJDIR)/parser_tests.o
OBJ_SEMANTIC_TESTS = $(OBJDIR)/semantic_tests.o
OBJ_LEXER_TESTS    = $(OBJDIR)/lexer_tests.o
OBJ_AST_TESTS      = $(OBJDIR)/ast_tests.o
OBJ_TEST_MAIN      = $(OBJDIR)/test_main.o

all: $(BINDIR)/cmmc $(BINDIR)/test_suite $(BINDIR)/assembly_tests $(BINDIR)/ir_tests $(BINDIR)/parser_tests $(BINDIR)/semantic_tests $(BINDIR)/lexer_tests $(BINDIR)/ast_tests

# Build compiler main executable
$(BINDIR)/cmmc: $(OBJDIR)/cmmc.o $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build main test suite executable (aggregated) - exclude test files with main()
$(BINDIR)/test_suite: $(OBJ_TEST_MAIN) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

# Build individual test executables
$(BINDIR)/assembly_tests: $(OBJ_ASSEMBLY_TESTS) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

$(BINDIR)/ir_tests: $(OBJ_IR_TESTS) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

$(BINDIR)/parser_tests: $(OBJ_PARSER_TESTS) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

$(BINDIR)/semantic_tests: $(OBJ_SEMANTIC_TESTS) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

$(BINDIR)/lexer_tests: $(OBJ_LEXER_TESTS) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

$(BINDIR)/ast_tests: $(OBJ_AST_TESTS) $(filter-out $(OBJDIR)/main.o, $(MAIN_OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_TEST)

# Compile object files for source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test_main.o with UNIT_TEST_MAIN defined
$(OBJDIR)/test_main.o: $(TESTDIR)/test_main.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST_MAIN -c $< -o $@

# Compile other test object files normally (without UNIT_TEST_MAIN)
$(OBJDIR)/%.o: $(TESTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test targets to run each test executable
test: assembly-tests ir-tests parser-tests semantic-tests lexer-tests ast-tests
	@echo "All tests completed."

test-suite: $(BINDIR)/test_suite
	@echo "Running aggregated test_suite:"
	@./$(BINDIR)/test_suite

assembly-tests: $(BINDIR)/assembly_tests
	@echo "Running assembly_tests:"
	@./$(BINDIR)/assembly_tests

ir-tests: $(BINDIR)/ir_tests
	@echo "Running ir_tests:"
	@./$(BINDIR)/ir_tests

parser-tests: $(BINDIR)/parser_tests
	@echo "Running parser_tests:"
	@./$(BINDIR)/parser_tests

semantic-tests: $(BINDIR)/semantic_tests
	@echo "Running semantic_tests:"
	@./$(BINDIR)/semantic_tests

lexer-tests: $(BINDIR)/lexer_tests
	@echo "Running lexer_tests:"
	@./$(BINDIR)/lexer_tests

ast-tests: $(BINDIR)/ast_tests
	@echo "Running ast_tests:"
	@./$(BINDIR)/ast_tests

clean:
	rm -rf $(OBJDIR) $(BINDIR) examples

# Help target
help:
	@echo "C-- Compiler Build System"
	@echo "========================"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build compiler and all test executables"
	@echo "  test             - Run all tests (each separately)"
	@echo "  test-suite       - Run aggregated test suite executable"
	@echo "  assembly-tests   - Run assembly tests"
	@echo "  ir-tests         - Run IR tests"
	@echo "  parser-tests     - Run parser tests"
	@echo "  semantic-tests   - Run semantic tests"
	@echo "  lexer-tests      - Run lexer tests"
	@echo "  ast-tests        - Run AST tests"
	@echo "  clean            - Remove build artifacts"
	@echo "  help             - Show this help text"

.PHONY: all clean test test-suite assembly-tests ir-tests parser-tests semantic-tests lexer-tests ast-tests help
