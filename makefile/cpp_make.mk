
TARGET_NAME      ?= a.out

ROOT_DIR         ?= $(shell pwd)
OBJ_DIR          ?= $(ROOT_DIR)/objs
SRC_DIR          ?= $(ROOT_DIR)/src
BIN_DIR          ?= $(ROOT_DIR)/bin
INCLUDE_DIR      ?= $(ROOT_DIR)/include


CC               ?= g++
CFLAGS           ?= 
LDFLAGS          ?= 
INCLUDES         ?= -I$(INCLUDE_DIR)/ -I$(SRC_DIR)/
LINK_LIBS_DIR    ?= 
LINK_LIBS        ?= 

C_SRC              ?= $(wildcard $(SRC_DIR)/*.cpp)
C_OBJS             := $(patsubst $(SRC_DIR)%.cpp, $(OBJ_DIR)%.o , $(C_SRC))


$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(OBJ_DIR))


$(BIN_DIR)/$(TARGET_NAME) : $(C_OBJS)
	$(CC)  $(LDFLAGS)  -o  $(BIN_DIR)/$(TARGET_NAME)  $(C_OBJS)  $(LINK_LIBS_DIR)  $(LINK_LIBS)

$(OBJ_DIR)/%.o  :  $(SRC_DIR)/%.cpp 
	$(CC)  -c  $(CFLAGS)  $<  -o  $@  $(INCLUDES)

.PHONY : clean
clean: 
	rm -f $(OBJ_DIR)/*.o 
	rm -f $(BIN_DIR)/*
