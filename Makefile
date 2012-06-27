CC      = gcc
# activate MODET because otherwise we would need to recompile libsim.a
CFLAGS  = -DMODET
LDFLAGS = -lbfd -lintl -liberty
GDBBUILDFOLDER = /d/build/gdb-7.4.1
SHAREDLIBRARYFLAGS = 

all: 
	$(CC) $(CFLAGS) armulmem.c -c -I$(GDBBUILDFOLDER)/sim/arm/
	$(CC) $(CFLAGS) sqGdbARMPlugin.c -c -fPIC -I$(GDBBUILDFOLDER)/sim/arm/
	gcc -shared $(SHAREDLIBRARYFLAGS) -o GdbARMPlugin.dll sqGdbARMPlugin.o armulmem.o libsim.a $(GDBBUILDFOLDER)/opcodes/libopcodes.a $(LDFLAGS)

