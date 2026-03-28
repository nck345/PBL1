# Dùng cmd.exe để đảm bảo lệnh if/mkdir chạy đúng trên Windows
SHELL = cmd.exe

# Bổ sung PATH để g++.exe tìm được các file DLL hệ thống của nó (phòng hờ)
export PATH := C:\msys64\ucrt64\bin;$(PATH)

# Trình biên dịch và Công cụ
CXX = C:/msys64/ucrt64/bin/g++.exe
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# File thực thi đầu ra
TARGET = $(BIN_DIR)/main.exe

# Flags cho trình biên dịch: -g (debug), -Iinclude (đường dẫn header)
CXXFLAGS = -g -I$(INC_DIR) -Wall

# Thư viện liên kết (FTXUI)
LDFLAGS = -lftxui-component -lftxui-dom -lftxui-screen

# Tìm toàn bộ file .cpp trong src và các thư mục con (repository, ui, services, utils)
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(SRC_DIR)/repository/*.cpp) \
        $(wildcard $(SRC_DIR)/services/*.cpp) \
        $(wildcard $(SRC_DIR)/ui/*.cpp) \
        $(wildcard $(SRC_DIR)/utils/*.cpp)

# Chuyển đổi danh sách file .cpp sang .o, dùng strip để tránh lỗi khoảng trắng
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(strip $(SRCS)))

# Target mặc định
all: $(TARGET)

# Bước Liên kết (Linker)
$(TARGET): $(OBJS)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Bước Biên dịch từng file (Compiler)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Dọn dẹp build
clean:
	@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	@if exist $(subst /,\,$(TARGET)) del /q $(subst /,\,$(TARGET))

.PHONY: all clean
