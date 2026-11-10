CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -g -Iinclude -MMD -MP

BUILD_DIR = build
EXEC = $(BUILD_DIR)/space-run
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: all
	./$(EXEC)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)

.PHONY: all run clean
