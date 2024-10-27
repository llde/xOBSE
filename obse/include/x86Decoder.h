#pragma once
#include <inttypes.h>

#define NONE 0xFFFFFFFF

#define REG_EAX 0
#define REG_ECX 1
#define REG_EDX 2
#define REG_EBX 3
#define REG_ESP 4
#define REG_EBP 5
#define REG_ESI 6
#define REG_EDI 7
#define REG_EPI 8
#ifdef REG_NONE
#undef REG_NONE 
#endif
#define REG_NONE NONE

#define CMD_NOP	 1
#define CMD_PUSH 2
#define CMD_CALL 3
#define CMD_XCHG 4

#define NEAR_REALTIVE_CALL 0
#define NEAR_ABSOLUTE_CALL 1

#define TYPE_REG8  1 
#define TYPE_REG32 2
#define TYPE_REG64 3
#define TYPE_NONE NONE

#define COMMAND			0
#define OPCODE			1
#define OPERAND0_TYPE	2
#define OPERAND0		3
#define OPERAND1_TYPE	4
#define OPERAND1		5

// Command	opcode	type   dest   type   source
static const unsigned int single_opcode_map[][6] =
{
	{NONE, NONE, TYPE_NONE, REG_NONE, TYPE_NONE, REG_NONE},

	{CMD_NOP, 0x90, TYPE_NONE, REG_NONE, TYPE_NONE, REG_NONE},

	{CMD_PUSH, 0x50, TYPE_REG32, REG_EAX, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x51, TYPE_REG32, REG_ECX, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x52, TYPE_REG32, REG_EDX, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x53, TYPE_REG32, REG_EBX, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x54, TYPE_REG32, REG_ESP, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x55, TYPE_REG32, REG_EBP, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x56, TYPE_REG32, REG_ESI, TYPE_NONE, REG_NONE},
	{CMD_PUSH, 0x57, TYPE_REG32, REG_EDI, TYPE_NONE, REG_NONE},

	{CMD_XCHG, 0x91, TYPE_REG32, REG_ECX, TYPE_REG32, REG_EAX},
	{CMD_XCHG, 0x92, TYPE_REG32, REG_EDX, TYPE_REG32, REG_EAX},
	{CMD_XCHG, 0x93, TYPE_REG32, REG_EBX, TYPE_REG32, REG_EAX},
	{CMD_XCHG, 0x94, TYPE_REG32, REG_ESP, TYPE_REG32, REG_EAX},
	{CMD_XCHG, 0x95, TYPE_REG32, REG_EBP, TYPE_REG32, REG_EAX},
	{CMD_XCHG, 0x96, TYPE_REG32, REG_ESI, TYPE_REG32, REG_EAX},
	{CMD_XCHG, 0x97, TYPE_REG32, REG_EDI, TYPE_REG32, REG_EAX}
};

typedef const unsigned int* singleOpcodeMap;

typedef struct sCallCmd
{
	unsigned int type;
	union
	{
		unsigned int call_r;
		struct
		{
			unsigned int reg;
			signed int offset;
		} callMPTR;
	} types;
} CallCommand, *PCallCommand;

/// <summary>
/// WARNING! This is not a complete decoder! It can fail in some cases
/// </summary>
/// <param name="retAddrs">The address of the next instruction relative to call</param>
/// <param name="pCallCmd">A pointer to CallCommand struct</param>
/// <returns>On success, it returns true</returns>
bool decodeCall(const void* const retAddrs, PCallCommand pCallCmd);
const singleOpcodeMap getSingleOpcodeMap(const unsigned char opcode);