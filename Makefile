CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src

SRC = src/lexer.cpp src/parser.cpp src/main.cpp

all: c_minus_compiler lexer_tests parser_tests ast_tests

c_minus_compiler: $(SRC)
	$(CXX) $(CXXFLAGS) -o c_minus_compiler $(SRC)

lexer_tests: tests/lexer_tests.cpp src/lexer.cpp
	$(CXX) $(CXXFLAGS) -o lexer_tests tests/lexer_tests.cpp src/lexer.cpp -lgtest -lgtest_main -pthread

parser_tests: tests/parser_tests.cpp src/lexer.cpp src/parser.cpp
	$(CXX) $(CXXFLAGS) -o parser_tests tests/parser_tests.cpp src/lexer.cpp src/parser.cpp -lgtest -lgtest_main -pthread

ast_tests: tests/ast_tests.cpp
	$(CXX) $(CXXFLAGS) -o ast_tests tests/ast_tests.cpp -lgtest -lgtest_main -pthread

test: lexer_tests parser_tests ast_tests

clean:
	rm -f c_minus_compiler lexer_tests parser_tests ast_tests