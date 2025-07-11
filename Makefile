UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  OPEN_CMD    := open
  LEAKS_CMD   := leaks -atExit --
else
  OPEN_CMD    := xdg-open
  LEAKS_CMD   := valgrind --leak-check=full --error-exitcode=1
endif

CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
LDFLAGS := -lncurses

TEST_CFLAGS   := -fprofile-arcs -ftest-coverage
TEST_LDFLAGS  := -lcheck

TEST_LDFLAGS := $(shell pkg-config --libs check)
TEST_CFLAGS   := $(shell pkg-config --cflags check)

SRC_DIRS := brick_game/tetris layer gui/cli
SRCS     := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))
HDRS     := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.h))
OBJDIR   := output
OBJS     := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
TARGET   := tetris_app

TEST_SRC       := tests/tests.c
TEST_TARGET       := run_tests
TEST_INCLUDES  := -Ibrick_game/tetris -Ilayer

prefix        = /usr/local
exec_prefix   = $(prefix)
bindir        = $(exec_prefix)/bin

COVERAGEDIR = $(OBJDIR)/coverage
ARCHIVE_NAME := tetris.tar.gz

# -------------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# -------------------------------------------------------------------
install: all
	mkdir -p $(bindir)
	install -m 0755 $(TARGET) $(bindir)/$(TARGET)

uninstall:
	rm -rf $(bindir)/$(TARGET)

# -------------------------------------------------------------------
ifeq ($(UNAME_S),Darwin)
test: $(TEST_SRC) brick_game/tetris/back.c layer/game.c
	mkdir -p $(OBJDIR) $(OBJDIR)/tests

	$(CC) $(CFLAGS) $(TEST_CFLAGS) $(TEST_INCLUDES) -o $(OBJDIR)/tests/$(TEST_TARGET) $(TEST_SRC) brick_game/tetris/back.c layer/game.c $(TEST_LDFLAGS)
	./$(OBJDIR)/tests/$(TEST_TARGET)

else
test: tests/tests.c brick_game/tetris/back.c layer/game.c
	mkdir -p $(OBJDIR) $(OBJDIR)/tests
	gcc $(TEST_CFLAGS) -o $(OBJDIR)/tests/$(TEST_TARGET) $^ $(TEST_LDFLAGS)
endif

gcov_report: test
	mkdir -p $(COVERAGEDIR) $(COVERAGEDIR)/report
	
	lcov \
	  --directory $(OBJDIR) \
	  --capture \
	  --output-file $(COVERAGEDIR)/coverage.info \
	  --ignore-errors empty,warning

	genhtml \
	  $(COVERAGEDIR)/coverage.info \
	  --output-directory $(COVERAGEDIR)/report

	$(OPEN_CMD) $(COVERAGEDIR)/report/index.html 2>/dev/null || true

# -------------------------------------------------------------------
ifeq ($(UNAME_S),Darwin)
dvi:
	mkdir -p output/docs
	doxygen Doxyfile
	$(OPEN_CMD) output/docs/html/index.html || true
else
dvi:
	doxygen Doxyfile 2>/dev/null
	@echo "HTML-доки лежат в docs/html/index.html"
endif

# -------------------------------------------------------------------
dist: clean all dvi test

	tar czvf $(OBJDIR)/$(ARCHIVE_NAME) \
	    --exclude-vcs \
	    --exclude='$(OBJDIR)' \
	    --exclude='$(TARGET)' \
	    .

# -------------------------------------------------------------------
checks:
	@echo "CLANG-FORMAT"
	clang-format -n $(SRCS) $(HDRS)
	clang-format -i $(SRCS) $(HDRS)
	
	@echo "\nCPPCHECK"
	@cppcheck --suppress=staticFunction --check-level=exhaustive --enable=all --suppress=missingIncludeSystem $(SRCS) tests/tests.c

leaks: test
	@$(LEAKS_CMD) $(OBJDIR)/tests/run_tests

# -------------------------------------------------------------------
clean:
	rm -rf $(OBJDIR) $(TARGET) record.txt