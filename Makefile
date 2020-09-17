CC = clang
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result
MUMSH_SRC = mumsh.c
TEST_SRC = test_exe.c
TEST = test
MUMSH = mumsh
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
MUMSHMC = mumsh_memory_check
.PHONY: clean

all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC) $(TEST)

test: clean
	@echo generating testing program...
	@$(CC) $(CFLAGS) -o $(TEST) $(TEST_SRC)
	@echo test successfully constructed, automatically running: 
	@./test