# Variables
CXX      := g++
CC       := gcc
CXXFLAGS := -std=c++23 -Wall -Wextra -O2 -Iinclude
CFLAGS   := -O2 -Iinclude
LDFLAGS  :=
LDLIBS   := -lm $(shell pkg-config --libs glfw3 gl)

# Directorios
SRC_DIR  := src
OBJ_DIR  := build/obj
BIN_DIR  := build

# Fuentes C++ y C por separado (recursivo en subdirectorios)
CXX_SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
C_SRCS   := $(shell find $(SRC_DIR) -name '*.c')

CXX_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CXX_SRCS))
C_OBJS   := $(patsubst $(SRC_DIR)/%.c,   $(OBJ_DIR)/%.o, $(C_SRCS))

OBJS     := $(CXX_OBJS) $(C_OBJS)
TARGET   := $(BIN_DIR)/cubo-opengl

# --- Reglas ---

# Target por defecto
all: $(TARGET)

# Linkado: todos los .o producen el ejecutable
# Copia de los shaders a la carpeta build
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	cp -r shaders $(BIN_DIR)/

# Compilación C++: cada .cpp produce su .o
# -MMD -MP genera los archivos .d de dependencias automáticas
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Compilación C: glad.c y otros .c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Incluir dependencias generadas (para recompilar cuando cambia un .h)
-include $(OBJS:.o=.d)

clean:
	rm -rf build/

.PHONY: all clean