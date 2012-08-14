CC      = gcc
CFLAGS  = 
LDFLAGS = libsim.a $(GDBBUILDFOLDER)/opcodes/libopcodes.a -lbfd -liberty -Wl,-wrap,ARMul_OSHandleSWI
GDBBUILDFOLDER = /home/lars/Documents/gsoc/gdb
SHAREDLIBRARYFLAGS = 

all:
	$(CC) $(CFLAGS) armulmem.c -c -I$(GDBBUILDFOLDER)/sim/arm/ 
	$(CC) $(CFLAGS) sqGdbARMPlugin.c -c -fPIC -I$(GDBBUILDFOLDER)/sim/arm/
	gcc -shared $(SHAREDLIBRARYFLAGS) -o GdbARMPlugin.dll sqGdbARMPlugin.o armulmem.o $(LDFLAGS)
	cp sqGdbARMPlugin.c GdbARMPlugin.h armulmem.c ../cog/platforms/Cross/plugins/GdbARMPlugin/
	cp Makefile.win32 ../cog/platforms/win32/plugins/GdbARMPlugin/Makefile
	cp Makefile.unix ../cog/platforms/unix/plugins/GdbARMPlugin/Makefile.inc
#	cd ../cog/unixbuild/bld; make; make install prefix=/home/lars/Documents/gsoc/vm

folders: 
#	mkdir ../cog/platforms/Cross/plugins/GdbARMPlugin
	mkdir ../cog/platforms/win32/plugins/GdbARMPlugin
	mkdir ../cog/platforms/unix/plugins/GdbARMPlugin

