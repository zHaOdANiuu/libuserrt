MAKEFLAGS += -rR

CC := clang
CFLAGS := -Wall -Werror -Wextra
LIBS := -lole32 -luuid -lntdll

cdos: cdos.c
	$(CC) $(CFLAGS) $< -o $@

auto-test:
	$@

fast-test: test.c
	$(CC) $(CFLAGS) $(LIBS) $< -o $@ && ./$@
