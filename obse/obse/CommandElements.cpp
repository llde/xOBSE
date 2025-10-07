#include "CommandTable.h"
// called from 004F90A5
bool Cmd_Default_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
#ifdef _DEBUG
#if 0
	_MESSAGE("Cmd_Default_Parse: %08X %08X %08X %08X",
		arg0, arg1, arg2, arg3);
#endif
#endif

#ifdef OBLIVION

	static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x004FDE30;

#else

#if CS_VERSION == CS_VERSION_1_0
	static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x004F69C0;
#elif CS_VERSION == CS_VERSION_1_2
	static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x00500FF0;
#else
#error unsupported cs version
#endif

#endif

	// arg0 = idx?
	// arg1 = ParamInfo *
	// arg2 = ptr to line to parse, skip UInt32 header first
	// arg3 = ptr to script info? first UInt32 is ptr to script data

	return g_defaultParseCommand(numParams, paramInfo, lineBuf, scriptBuf);
}


// nop command handler for script editor
bool Cmd_Default_Execute(COMMAND_ARGS)
{
	return true;
}
// nop command handler for script editor
bool Cmd_Default_Eval(COMMAND_ARGS_EVAL)
{
	return true;
}