# Target OS: linux, windows, macos (default: detect host)
ifeq ($(TARGET_OS),)
    ifeq ($(OS),Windows_NT)
        TARGET_OS = windows
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            TARGET_OS = macos
        else
            TARGET_OS = linux
        endif
    endif
endif

CC = gcc
AR = ar

ifeq ($(TARGET_OS),windows)
    CC = x86_64-w64-mingw32-gcc
    AR = x86_64-w64-mingw32-ar
endif

CFLAGS = -Wall -Wextra -O2 -Iinclude -Isrc -MMD -MP

SRC_DIR = src
BUILD_DIR = build/$(TARGET_OS)
TEST_DIR = src/test
TEST_SRC = $(TEST_DIR)/test_model.c
TEST_BIN = test_model

NAME = $(BUILD_DIR)/libgj_model.a

# Library sources only (exclude tests)
SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -path "$(TEST_DIR)/*")
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean fclean re test

all: $(NAME)

test: $(NAME)
	$(CC) $(CFLAGS) $(TEST_SRC) -L$(BUILD_DIR) -lgj_model -o $(TEST_BIN)

$(NAME): $(OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)
