
SRCS := $(wildcard *.c) helpers/bitmap.c helpers/blocks.c helpers/slist.c
OBJS := $(SRCS:.c=.o)
HDRS := $(wildcard *.h hints/*.h helpers/*.h)

CFLAGS := -g `pkg-config fuse --cflags` -Ihelpers -Ihints
LDLIBS := `pkg-config fuse --libs`

nufs: $(OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c $(HDRS)
	gcc $(CFLAGS) -c -o $@ $<

clean: unmount
	rm -f nufs *.o helpers/*.o test.log data.nufs
	rmdir mnt || true

mount: nufs
	mkdir -p mnt || true
	./nufs -s -f mnt data.nufs

unmount:
	fusermount -u mnt || true

test: nufs
	perl test.pl

gdb: nufs
	mkdir -p mnt || true
	gdb --args ./nufs -s -f mnt data.nufs

.PHONY: clean mount unmount gdb
