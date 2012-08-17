GDBBUILDFOLDER = /d/build/gdb-7.4

CC      = gcc
CFLAGS  = -I$(GBDBUILDFOLDER)/bfd 
LDFLAGS = $(GDBBUILDFOLDER)/sim/arm/libsim.a $(GDBBUILDFOLDER)/opcodes/libopcodes.a -lbfd -liberty -Wl,-wrap,ARMul_OSHandleSWI -L$(GDBBUILDFOLDER)/bfd/ 
SHAREDLIBRARYFLAGS = 

all:
	$(CC) $(CFLAGS) armulmem.c -c -I$(GDBBUILDFOLDER)/sim/arm/ -I$(GDBBUILDFOLDER)/include/
	$(CC) $(CFLAGS) sqGdbARMPlugin.c -c -fPIC -I$(GDBBUILDFOLDER)/sim/arm/ -I$(GDBBUILDFOLDER)/include/ -I$(GDBBUILDFOLDER)/bfd/ 
	$(CC) -shared $(SHAREDLIBRARYFLAGS) -o GdbARMPlugin.dll sqGdbARMPlugin.o armulmem.o $(LDFLAGS)

	mkdir -p ../cog/platforms/Cross/plugins/GdbARMPlugin
	mkdir -p ../cog/platforms/win32/plugins/GdbARMPlugin
	mkdir -p ../cog/platforms/unix/plugins/GdbARMPlugin

	cp sqGdbARMPlugin.c GdbARMPlugin.h armulmem.c ../cog/platforms/Cross/plugins/GdbARMPlugin/
	cp Makefile.win32 ../cog/platforms/win32/plugins/GdbARMPlugin/Makefile
	cp Makefile.unix ../cog/platforms/unix/plugins/GdbARMPlugin/Makefile.inc
#	cd ../cog/unixbuild/bld; make; make install prefix=/home/lars/Documents/gsoc/vm
