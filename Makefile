CFLAGS= -Wall -pedantic

SRC=${wildcard *.c}

OUT=$(SRC:.c=.out)
all: $(OUT)

%.out: %.c
	$(CC) $(CFLAGS) $< -o $@ -lacl
clean:
	rm -rf *.out
