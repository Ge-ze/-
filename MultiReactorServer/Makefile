# 头文件目录
INC_DIR := include
# 源文件目录
SRC_DIR := src
# 目标文件目录
OBJ_DIR := obj

# C++ 编译器和编译选项
CC := g++
CFLAGS := -c -Wall -I$(INC_DIR)

# 链接选项
LDFLAGS := -lpthread

# 源文件列表
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
# 目标文件列表
OBJECTS := $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

# 可执行文件名称
EXECUTABLE := server

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)
