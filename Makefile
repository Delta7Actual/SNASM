CC = gcc
CFLAGS = -Wall -Wextra -pedantic
INCLUDES = -Iinclude
SRC = $(wildcard src/*.c)
OBJDIR = build
OBJ = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
EXEC = SNASM
TEST_EXEC = SNASM_test

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

test: CFLAGS += -DTEST_MODE
test: $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TEST_EXEC) $^

$(OBJDIR)/%.o: src/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(EXEC) $(TEST_EXEC) output

format:
	clang-format -i src/*.c include/*.h

.PHONY: all test clean format