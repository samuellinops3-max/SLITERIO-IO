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
TITLE_ID   = PS3SLITHE
TITLE      = PS3 Slither
VERSION    = 01.00

all: $(SELF)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(SELF): $(ELF)
	make_self_npdrm $(ELF) $(SELF) $(CONTENT_ID)

pkg: $(SELF)
	mkdir -p pkg_build/USRDIR pkg_build/TROPDIR
	cp $(SELF) pkg_build/USRDIR/EBOOT.BIN
	psn_package_npdrm <(echo "CONTENT_ID=$(CONTENT_ID)
TITLE=$(TITLE)
TITLE_ID=$(TITLE_ID)
VERSION=$(VERSION)
CATEGORY=HG") pkg_build/
	package_finalize $(CONTENT_ID).pkg
	mv $(CONTENT_ID).pkg PS3-Slither.pkg

clean:
	rm -f $(OBJ) $(ELF) $(SELF)
	rm -rf pkg_build *.pkg
