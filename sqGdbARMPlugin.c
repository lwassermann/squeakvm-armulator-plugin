#define COG 1
#define FOR_COG_PLUGIN 1

//disassembler
#include <bfd.h>
#include <dis-asm.h>

#include <stdarg.h>

//emulator
#include <armdefs.h>
#include <armemu.h>

#include "GdbARMPlugin.h"

ARMul_State*	lastCPU;


// These two variables exist, in case there are library-functions which write to a stream.
// In that case, we would write functions which print to that stream instead of stderr or similar
#define LOGSIZE 4096
static char	gdb_log[LOGSIZE+1];
static int	gdblog_index = 0;

ulong	minReadAddress, minWriteAddress;

// what is that for?
	   void			(*prevInterruptCheckChain)() = 0;

void*
newCPU()
{
	lastCPU = ARMul_NewState();
	return lastCPU;
}

int
resetCPU(void* cpu)
{
	unsigned int i, j;
	ARMul_State* state = (ARMul_State*) cpu;
	// test whether the supplied instance is an ARMul type?
	
	gdblog_index = 0;
	
	// reset registers in all modes
	for (i = 0; i < 16; i++)
	{
		state->Reg[i] = 0;
		for (j = 0; j < 7; j++)
			state->RegBank[j][i] = 0;
	}
	for (i = 0; i < 7; i++)
		state->Spsr[i] = 0;
	
	ARMul_Reset(state);
	return 0;
}

int
singleStepCPUInSizeMinAddressReadWrite(void *cpu, void *memory, 
		ulong byteSize, ulong minAddr, ulong minWriteMaxExecAddr)
{
	ARMul_State* state = (ARMul_State*) cpu;
	lastCPU = state;
	
	// test whether the supplied instance is an ARMul type?
	state->MemDataPtr = (unsigned char*) memory;
	state->MemSize = byteSize;
	minReadAddress = minAddr;
	minWriteAddress = minWriteMaxExecAddr;
	
	gdblog_index = 0;
	
	state->EndCondition = NoError;
	state->Reg[15] = ARMul_DoInstr(state);
	
	if(state->EndCondition != NoError){
		return state->EndCondition;
	}
	
	return gdblog_index == 0 ? 0 : SomethingLoggedError;
}

int
runCPUInSizeMinAddressReadWrite(void *cpu, void *memory, 
		ulong byteSize, ulong minAddr, ulong minWriteMaxExecAddr)
{
	ARMul_State* state = (ARMul_State*) cpu;
	lastCPU = state;
	
	// test whether the supplied instance is an ARMul type?
	state->MemDataPtr = (unsigned char*) memory;
	state->MemSize = byteSize;
	minReadAddress  = minAddr;
	minWriteAddress = minWriteMaxExecAddr;
	
	gdblog_index = 0;
	
	state->EndCondition = NoError;
	state->Reg[15] = ARMul_DoProg(state);
	
	if(state->EndCondition != NoError){
		return state->EndCondition;
	}
	
	return gdblog_index == 0 ? 0 : SomethingLoggedError;
}

// next functions reason for existence is not clear
/*
 * Currently a dummy for Bochs.
 */
void
flushICacheFromTo(void *cpu, ulong saddr, ulong eaddr)
{
#if 0
# error not yet implemented
#endif
}

int
gdb_log_sprintf(void* stream, const char * format, ...)
{
	va_list arg;
	
	va_start(arg,format);
	vsnprintf((char*) (&gdb_log) + gdblog_index, LOGSIZE-gdblog_index, format, arg);
	return 0;
}

int
disassembleForAtInSize(void *cpu, ulong laddr,
			void *memory, ulong byteSize)
{
	// ignore the cpu
	// start disassembling at laddr relative to memory
	// stop disassembling at memory+byteSize
	
	disassemble_info* dis = (disassemble_info*) calloc(1, sizeof(disassemble_info));
	// void init_disassemble_info (struct disassemble_info *dinfo, void *stream, fprintf_ftype fprintf_func)
	init_disassemble_info ( dis, NULL, gdb_log_sprintf);
	
	dis->arch = bfd_arch_arm;
	dis->mach = bfd_mach_arm_unknown;
	
	// sets some fields in the structure dis to architecture specific values
	disassemble_init_for_target( dis );
	// given a bfd, the disassembler can find the arch by itself.
	// unfortunately, we don't have bfd-structures, so we have to choose the function by hand.
	//disassemble = disassembler( c );
	disassembler_ftype disassembler = print_insn_little_arm;
	//other possible functions are listed in opcodes/dissassemble.c
	
	dis->buffer_vma = 0;
	dis->buffer = memory;
	dis->buffer_length = byteSize;
	
	// while-loop for calling single instruction decoding:
	unsigned int isize = 0;
	size_t pos = laddr;
	size_t max_pos = dis->buffer_vma+dis->buffer_length;
	
	while(pos < max_pos)
	  {
	    unsigned int size = disassembler((bfd_vma) pos, dis);
	    pos += size;
	    isize++;
	    gdb_log_sprintf(NULL, "\n");
	  }
	free(dis);
	gdb_log[gdblog_index+1] = 0;
	return isize;
}

void
forceStopRunning()
{
	lastCPU->Emulate = STOP;
}

int
errorAcorn(void) { return 0; }

char *
getlog(long *len)
{
	*len = gdblog_index;
	return gdb_log;
}

/*
void
initialiseModule()
{
	ARMul_EmulateInit();
}

void
shutdownModule()
{
	// nothing to do so far.
	// maybe free the cpu, once that turns into a singleton
}

void
setInterpreter(int interpreterProxy)
{
	// not needed, so the interpreterProxy is not saved in a static var
}
*/
