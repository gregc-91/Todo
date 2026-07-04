
# Config options
EXE    = todo
SRC    = src
BIN    = bin
CXX      ?= c++
CXXFLAGS ?= -O3 -g
CXXFLAGS += -Wall -Wextra -std=c++17
SHELL  = /bin/bash

# Shell colours
BLACK =\u001b[30;1m
RED   =\u001b[31;1m
GREEN =\u001b[32;1m
YELLOW=\u001b[33;1m
BLUE  =\u001b[34;1m
PINK  =\u001b[35;1m
CYAN  =\u001b[36;1m
WHITE =\u001b[37;1m
RESET =\u001b[0m

# List all the header files
HDRS := $(wildcard $(SRC)/*.h)

# List all the object files
SRCS := $(wildcard $(SRC)/*.cpp)
OBJS := $(patsubst $(SRC)/%.cpp,$(BIN)/obj/%.o,$(SRCS))

# Rules
.PHONY: clean
.SILENT: $(BIN)/$(EXE) $(OBJS) $(BIN) clean

# Rule to create the executable
$(BIN)/$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) && \
	printf "Make: successfully built executable ${CYAN}%s${RESET}\n" "$@"

# Rule to create the object files
$(BIN)/obj/%.o: $(SRC)/%.cpp $(HDRS) | $(BIN)
	printf "Make: compiling source file ${PINK}%s${RESET}\n" "$<" && \
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Rule to create the output folder
$(BIN):
	mkdir -p $(BIN)/obj

# Delete the objects and executable
clean:
	printf "Make: cleaning up\n" && \
	rm -rf $(BIN)
