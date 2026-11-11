PS3DEV   ?= $(HOME)/ps3dev
PSL1GHT  ?= $(PS3DEV)
CC        = ppu-gcc
CFLAGS    = -mcpu=cell -O2 -I$(PSL1GHT)/include
LDFLAGS   = -L$(PSL1GHT)/lib -lgcm_sys -lrsx -lpad -lsysutil -lsysmodule -lm

TARGET    = PS3_SLITHER
ELF       = $(TARGET).elf
SELF      = EBOOT.BIN
SRC       = source/main.c
OBJ       = $(SRC:.c=.o)

CONTENT_ID = UP0001-PS3SLITHE_00-0000000000000000

all: $(SELF)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(SELF): $(ELF)
	make_self_npdrm $(ELF) $(SELF) $(CONTENT_ID)

clean:
	rm -f $(OBJ) $(ELF) $(SELF)
