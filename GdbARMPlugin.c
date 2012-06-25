#define COG 1
#define FOR_COG_PLUGIN 1

// Enables Thumb-Mode in the ARMulator. This is defined by default when compiling libsim.a and adds a field to ARMul_State
#define MODET 

//disassembler
#include <bfd.h>
#include <dis-asm.h>

//emulator
#include <armdefs.h>
#include <armemu.h>

#include "GdbARMPlugin.h"

ARMul_State*	state;

static int	hasInitialized = 0;

#define LOGSIZE 4096
static char	gdb_log[LOGSIZE+1];
static int	gdblog_index = 0;

static unsigned char*	theMemory = 0;
static ulong	theMemorySize;
static ulong	minReadAddress;
static ulong	minWriteAddress;

void*
newCPU()
{
	if(!hasInitialized){
		ARMul_EmulateInit();
		hasInitialized = 1;
	}
	return ARMul_NewState();
}

int
resetCPU(void* cpu)
{
	ARMul_State* state = (ARMul_State*) cpu;
	// test whether the supplied instance is an ARMul type?
	
	gdblog_index = 0;
	ARMul_Reset(state);
	return 0;
}

int
singleStepCPUInSizeMinAddressReadWrite(void *cpu,
		void *memory, ulong byteSize, ulong minAddr, ulong minWriteMaxExecAddr)
{
	ARMword pc = 0;
	ARMul_State* state = (ARMul_State*) cpu;
	// test whether the supplied instance is an ARMul type?
	
	theMemory = (unsigned char*) memory;
	theMemorySize = byteSize;
	minReadAddress = minAddr;
	minWriteAddress = minWriteMaxExecAddr;
	
	gdblog_index = 0;
	
	ARMul_SetPC(state, (ARMword) memory);
	pc = ARMul_DoInstr(state);
	return gdblog_index == 0 ? 0 : SomethingLoggedError;
}

int
runCPUInSizeMinAddressReadWrite(void *cpu, void *memory, ulong byteSize,
								ulong minAddr, ulong minWriteMaxExecAddr)
{
	ARMul_State* state = (ARMul_State*) cpu;

	// test whether the supplied instance is an ARMul type?
	theMemory = (unsigned char *)memory;
	theMemorySize = byteSize;
	minReadAddress = minAddr;
	minWriteAddress = minWriteMaxExecAddr;
	
	gdblog_index = 0;
	
	ARMul_SetPC(state, (ARMword) memory);
	pc = ARMul_DoProg(state);
	// fill the errorAcorn?
	
	return gdblog_index == 0 ? 0 : SomethingLoggedError;
}

// next functions reason for existence is not clear
/*
 * Currently a dummy for Bochs.
 */
void
flushICacheFromTo(void *cpu, ulong saddr, ulong eaddr)
{
#if BX_SUPPORT_ICACHE
# error not yet implemented
#endif
}


int
disassembleForAtInSize(void *cpu, ulong laddr,
						void *memory, ulong byteSize)
{
	ARMul_State* state = (ARMul_State*) cpu;

	Bit8u  instr_buf[16];
	size_t i=0;

	static char letters[] = "0123456789ABCDEF";
	static disassembler bx_disassemble;
	long remainsInPage = byteSize - laddr;

	if (remainsInPage < 0) {
		theErrorAcorn = MemoryBoundsError;
		return -MemoryBoundsError;
	}

	memcpy(instr_buf, (char *)memory + laddr, min(15,byteSize - laddr));
	i = sprintf(bochs_log, "%08lx: ", laddr);
	bx_disassemble.set_syntax_att();
	unsigned isize = bx_disassemble.disasm(
						anx86->sregs[BX_SEG_REG_CS].cache.u.segment.d_b,
						anx86->cpu_mode == BX_MODE_LONG_64,
						anx86->get_segment_base(BX_SEG_REG_CS), laddr,
						instr_buf,
						bochs_log+i);
	if (isize <= remainsInPage) {
	  i=strlen(bochs_log);
	  bochs_log[i++] = ' ';
	  bochs_log[i++] = ':';
	  bochs_log[i++] = ' ';
	  for (unsigned j=0; j<isize; j++) {
		bochs_log[i++] = letters[(instr_buf[j] >> 4) & 0xf];
		bochs_log[i++] = letters[(instr_buf[j] >> 0) & 0xf];
		bochs_log[i++] = ' ';
	  }
	}
	bochs_log[blidx = i] = 0;
	return isize;
}

void
forceStopRunning(void)
{
	if (prevInterruptCheckChain)
		prevInterruptCheckChain();
	bx_pc_system.kill_bochs_request = 1;
	bx_cpu.async_event = 1;
}

int
errorAcorn(void) { return theErrorAcorn; }

char *
getlog(long *len)
{
	*len = blidx;
	return bochs_log;
}

