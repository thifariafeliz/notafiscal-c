CC = gcc

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BLD_DIR = build

SRCS = $(SRC_DIR)/main.c    \
	$(SRC_DIR)/list.c       \
	$(SRC_DIR)/utils.c      \
	$(SRC_DIR)/stringo.c    \
	$(SRC_DIR)/notafiscal.c \
	$(SRC_DIR)/operacoes_clientes.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CFLAGS_BASE = -std=c23 -Wall -Wextra -Wpedantic -Werror -Wshadow \
	-Wconversion -Wsign-conversion -Wformat=2 -Wnull-dereference \
	-fstack-protector-strong -D_FORTIFY_SOURCE=2 -I$(INC_DIR)

CFLAGS_DEBUG = -g3 -O2 -fsanitize=address,undefined -fno-omit-frame-pointer -ggdb3

CFLAGS_RELEASE = -O2 -DNDEBUG

LDFLAGS = -lasan -Wl,-z,relro,-z,now  -fsanitize=address,undefined

ifdef debug
	CFLAGS = $(CFLAGS_BASE) $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_BASE) $(CFLAGS_RELEASE)
endif

TARGET = build/nf

all: $(TARGET)

$(OBJ_DIR):
	@mkdir -p $@

$(BLD_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) | $(BLD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	@rm -rf ./$(OBJ_DIR) ./$(BLD_DIR)
