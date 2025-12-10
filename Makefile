# Prefer the course-required compiler version (Ubuntu 22.04 ships g++ 11.4.0).
# Falls back to the default g++ if g++-11 is not available.
CXX ?= $(shell if command -v g++-11 >/dev/null 2>&1; then echo g++-11; else echo g++; fi)

CXXFLAGS ?= -std=c++17 -Wall -Wextra -I.

LIB_SRCS := AnalisadorLexico.cpp AnalisadorSintatico.cpp AnalisadorSemantico.cpp AnalisadorGCI.cpp
COMPILADOR_BIN := compilador

TEST_BIN_DIR := build/tests
TEST_SOURCES := \
	testes/test_lexico_basico.cpp \
	testes/test_semantico_basico.cpp \
	testes/test_pipeline_validos.cpp \
	testes/test_pipeline_invalidos.cpp

TEST_BINS := $(patsubst testes/%.cpp,$(TEST_BIN_DIR)/%,$(TEST_SOURCES))

.PHONY: all tests run-compilador clean

all: tests run-compilador

$(COMPILADOR_BIN): compilador.cpp $(LIB_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_BIN_DIR):
	mkdir -p $(TEST_BIN_DIR)

$(TEST_BIN_DIR)/%: testes/%.cpp $(LIB_SRCS) | $(TEST_BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(LIB_SRCS) -o $@

tests: $(TEST_BINS)
	@set -e; \
	for t in $(TEST_BINS); do \
		echo "==== Running $$t ===="; \
		$$t; \
	done

# Roda o compilador nos 3 programas válidos via menu automatizado
run-compilador: $(COMPILADOR_BIN)
	@echo "==== Running compilador (programas válidos) ===="
	@printf '1\n2\n3\n0\n' | ./$(COMPILADOR_BIN)

clean:
	rm -rf $(TEST_BIN_DIR) $(COMPILADOR_BIN)
