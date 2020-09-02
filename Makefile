.PHONY: test parsergen

CFLAGS := -O3 -std=c11 -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS += -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch -pthread
LDFLAGS :=

OBJS := \
	tsfind.o \
	argparse.o \
	pgmopts.o \
	audio_extract.o

all: tsfind

clean:
	rm -f $(OBJS)
	rm -f tsfind

tsfind: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: all
	./tsfind example.wav

parsergen:
	../pypgmopts/pypgmopts parser.py

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
