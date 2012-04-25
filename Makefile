MAKETOPDIR="$(shell pwd)"
export MAKETOPDIR
include Makefile.common
LDFLAGS=$(COMMONFLAGS) -fno-exceptions -ffunction-sections -fdata-sections -L$(LIBDIR) -nostartfiles -Wl,--gc-sections,-Tstm32f1x_md.ld

LDLIBS+=-lm
LDLIBS+=-lstm32

STARTUP=startup.c
all: libs src
	$(CC) -o $(PROGRAM).elf $(LDFLAGS) \
		-Wl,--whole-archive \
			src/app.a \
		-Wl,--no-whole-archive \
			$(LDLIBS)
	$(OBJCOPY) -O ihex   $(PROGRAM).elf $(PROGRAM).hex
	$(OBJCOPY) -O binary $(PROGRAM).elf $(PROGRAM).bin

.PHONY: libs src clean
libs:
	$(MAKE) -C libs $@

src:
	$(MAKE) -C src $@ 

clean:
	$(MAKE) -C src $@
	$(MAKE) -C libs $@
	rm -f $(PROGRAM).{elf,hex,bin}
