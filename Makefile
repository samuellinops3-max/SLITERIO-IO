PS3DEV   ?= /usr/local/ps3dev
PSL1GHT  ?= $(PS3DEV)
CC        = ppu-gcc
CFLAGS    = -mcpu=cell -O2 -I$(PSL1GHT)/include
LDFLAGS   = -L$(PSL1GHT)/lib -lgcm_sys -lrsx -lpad -lsysutil -lsysmodule -lm

SRC       = source/main.c
OBJ       = $(SRC:.c=.o)

all: EBOOT.BIN

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

EBOOT.BIN: $(OBJ)
	$(CC) $(CFLAGS) -o PS3_SLITHER.elf $^ $(LDFLAGS)
	make_self_npdrm PS3_SLITHER.elf EBOOT.BIN UP0001-PS3SLITHE_00-0000000000000000

clean:
	rm -f *.o *.elf EBOOT.BIN>
