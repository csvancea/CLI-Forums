CXXFLAGS = -c -Wall -Wextra -std=c++17 -DFMT_HEADER_ONLY -I./include
# CXXFLAGS += -g -DENABLE_LOGGING
LDFLAGS =

IP ?= 127.0.0.1
PORT ?= 7331
CLIENT_ID ?= hello

EXE_SERVER = server
EXE_CLIENT = subscriber

SRC_DIR = src
OUT_DIR = build
OBJ_DIR = $(OUT_DIR)/obj

OUT_EXE_SERVER = $(OUT_DIR)/$(EXE_SERVER)
OUT_EXE_CLIENT = $(OUT_DIR)/$(EXE_CLIENT)

SRC_FILES_COMMON := $(shell find $(SRC_DIR)/common/ -type f -name '*.cpp')
OBJ_FILES_COMMON := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES_COMMON))

SRC_FILES_SERVER := $(shell find $(SRC_DIR)/server/ -type f -name '*.cpp')
OBJ_FILES_SERVER := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES_SERVER))

SRC_FILES_CLIENT := $(shell find $(SRC_DIR)/client/ -type f -name '*.cpp')
OBJ_FILES_CLIENT := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES_CLIENT))

SRC_FILES_SERVER := $(SRC_FILES_SERVER) $(SRC_FILES_COMMON)
SRC_FILES_CLIENT := $(SRC_FILES_CLIENT) $(SRC_FILES_COMMON)

OBJ_FILES_SERVER := $(OBJ_FILES_SERVER) $(OBJ_FILES_COMMON)
OBJ_FILES_CLIENT := $(OBJ_FILES_CLIENT) $(OBJ_FILES_COMMON)


.PHONY: build
build: build_server build_client

.PHONY: run_server
run_server: build_server
	./$(OUT_EXE_SERVER) $(PORT)

.PHONY: run_client
run_client: build_client
	./$(OUT_EXE_CLIENT) $(CLIENT_ID) $(IP) $(PORT)

.PHONY: clean
clean:
	rm -rf "$(OUT_DIR)" "$(OBJ_DIR)"

.PHONY: build_server
build_server: $(OUT_EXE_SERVER)

.PHONY: build_client
build_client: $(OUT_EXE_CLIENT)

$(OUT_EXE_SERVER): $(OBJ_FILES_SERVER)
	@mkdir -p "$(OUT_DIR)"
	@echo Linking "$(OUT_EXE_SERVER)" ...
	@$(CXX) $(LDFLAGS) -o "$(OUT_EXE_SERVER)" $^

$(OUT_EXE_CLIENT): $(OBJ_FILES_CLIENT)
	@mkdir -p "$(OUT_DIR)"
	@echo Linking "$(OUT_EXE_CLIENT)" ...
	@$(CXX) $(LDFLAGS) -o "$(OUT_EXE_CLIENT)" $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p "$(@D)"
	@echo Compiling "$<" ...
	@$(CXX) $(CXXFLAGS) -o $@ $<
