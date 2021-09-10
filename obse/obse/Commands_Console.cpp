#include "Commands_Console.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#ifdef OBLIVION

#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "Hooks_Gameplay.h"

/* Print formatted string to Oblivion console
 * syntax: PrintToConsole fmtstring num1 num2 ...
 * shortname: printc
 *
 * Prints a formatted string to the Oblivion console, similar to how printf() works.
 * -Format notation is the same as MessageBox, so you can use %#.#f %g %e %%
 * -While this function technically accepts floats, you can use %g to print integers.  Again, how this works
 *  is probably similar to how MessageBox works (ie. same range limitations).
 * -The string can be up to 511 characters long, not including the null byte.
 * -You can pass up to 9 variables, but you don't have to pass any at all.
 *
 * Updated v0014: Takes additional format specifiers as MessageEX/MessageBoxEX
 * retains previous functionality
 */
bool Cmd_PrintToConsole_Execute(COMMAND_ARGS)
{
	*result = 0;
	char buffer[kMaxMessageLength];

	if (ExtractFormatStringArgs(0, buffer, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_PrintToConsole.numParams))
	{
		if (strlen(buffer) < 512)
		{
			*result = 1;
			Console_Print(buffer);
		}
		else
			Console_Print_Long(buffer);
	}

	return true;
}

/* Print formatted string to the given file
 * syntax:  PrintToFile fmtstring num1 num1 ...
 * shortname: printf
 *
 * Prints a formatted string to the given file, similar to using PrintToConsole.
 * -You can use integers here like with PrintToConsole and MessageBox.
 * -The string can be up to 511 character long, not including the null byte.
 * -You can pass up to 9 variables, but you don't have to pass any at all.
 * -If the file does not exist, it will be created.
 * -This function will always append to the end of a file on a new line.
 */
bool Cmd_PrintToFile_Execute(COMMAND_ARGS)
{
	char filename[129], fmtstring[BUFSIZ];
	FILE* fileptr;
	float f0, f1, f2, f3, f4, f5, f6, f7, f8;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &filename, &fmtstring, &f0, &f1, &f2, &f3, &f4,
					&f5, &f6, &f7, &f8))
	{
		fopen_s (&fileptr, filename, "a+");
		strcat_s(fmtstring, sizeof(fmtstring), "\n");
		fprintf (fileptr, fmtstring, f0, f1, f2, f3, f4, f5, f6, f7, f8);
		fclose (fileptr);
	}
	return true;
}

bool RunScriptLine2(const char * buf, bool bSuppressOutput)
{
	// we have to retain the original RunScriptLine() (w/o output suppression option) for plugins
	ToggleConsoleOutput(!bSuppressOutput);

	bool bResult = RunScriptLine(buf);

	ToggleConsoleOutput(true);
	return bResult;
}

bool RunScriptLine(const char* buf)
{
	// create a Script object
	UInt8	scriptObjBuf[sizeof(Script)];
	Script	* tempScriptObj = (Script *)scriptObjBuf;

	void	* scriptState = GetGlobalScriptStateObj();
	bool bResult = false;

	if (scriptState && *((void**)scriptState))		// ### need to add a guard as the state object can be NULL sometimes (no idea why)
	{
		tempScriptObj->Constructor();
		tempScriptObj->MarkAsTemporary();
		tempScriptObj->SetText(buf);
		bResult = tempScriptObj->CompileAndRun(*((void**)scriptState), 1, NULL);
		tempScriptObj->StaticDestructor();
	}

	return bResult;
}

bool RunScriptLineOnREFR(const char * buf, TESObjectREFR* callingObj, bool bSuppressOutput)
{
	ToggleConsoleOutput(!bSuppressOutput);

	// create a Script object
	UInt8	scriptObjBuf[sizeof(Script)];
	Script	* tempScriptObj = (Script *)scriptObjBuf;

	void	* scriptState = GetGlobalScriptStateObj();
	bool bResult = false;

	if (scriptState && *((void**)scriptState))
	{
		tempScriptObj->Constructor();
		tempScriptObj->MarkAsTemporary();
		tempScriptObj->SetText(buf);
		bResult = tempScriptObj->CompileAndRun(*((void**)scriptState), 1, callingObj);
		tempScriptObj->StaticDestructor();
	}

	ToggleConsoleOutput(true);
	return bResult;
}

bool Cmd_RunBatchScript_Execute(COMMAND_ARGS)
{
	char	fileName[1024];
	UInt32	bRunOnRef = 0;
	UInt32	bSuppressOutput = 0;

	*result = 0;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &fileName, &bRunOnRef, &bSuppressOutput)) {
		return true;
	}

	FILE	* src = NULL;
	fopen_s(&src, fileName, "r");
	if(src)
	{
		*result = 1;

		char	line[4096];
		while(fgets(line, sizeof(line), src))
		{
			UInt32	lineLen = strlen(line);
			if(lineLen > 1)	// skip empty lines
			{
				// strip the trailing newline (if we have one)
				if(line[lineLen - 1] == '\n')
					line[lineLen - 1] = 0;

				if (bRunOnRef)
				{
					if (!RunScriptLineOnREFR(line, thisObj, bSuppressOutput ? true : false))
						*result = 0;
				}
				else
				{
					if (!RunScriptLine2(line, bSuppressOutput ? true : false))
						*result = 0;
				}
			}
		}

		fclose(src);
	}

	return true;
}

static bool Cmd_RunScriptLine_Execute(COMMAND_ARGS)
{
	char scriptText[kMaxMessageLength];
	UInt32 bRunOnRef = 0;
	UInt32 bSuppressOutput = 0;
	*result = 0;

	if (!ExtractFormatStringArgs(0, scriptText, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_RunScriptLine.numParams, &bRunOnRef, &bSuppressOutput))
		return true;

	UInt32 len = strlen(scriptText);
	if (len)
	{
		if (bRunOnRef)
			*result = RunScriptLineOnREFR(scriptText, thisObj, bSuppressOutput ? true : false) ? 1 : 0;
		else
			*result = RunScriptLine2(scriptText, bSuppressOutput ? true : false) ? 1 : 0;
	}

	return true;
}

static Bitfield<UInt32> ModDebugStates[8];

static bool Cmd_SetDebugMode_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 bEnableDebug = 0;
	UInt32 modIndexArg = 0xFFFF;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &bEnableDebug, &modIndexArg))
		return true;

	UInt8 modIndex = modIndexArg;
	if (modIndexArg == 0xFFFF)
		modIndex = scriptObj->GetModIndex();

	if (modIndex > 0 && modIndex < 0xFF)
	{
		UInt8 modBit = modIndex % 32;			//which bit to toggle
		//modIndex /= 32;
		UInt8 bucket = modIndex / 32;			//index into bitfield array
		if (bEnableDebug)
			ModDebugStates[bucket].Set(1 << modBit);
		else
			ModDebugStates[bucket].UnSet(1 << modBit);

		if (IsConsoleMode())
			Console_Print("Debug statements toggled %s for mod %02X", (bEnableDebug ? "on" : "off"), modIndex);
	}

	return true;
}

static bool Cmd_DebugPrint_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt8 modIndex = scriptObj->GetModIndex();
	UInt8 modBit = modIndex % 32;
	modIndex /= 32;

	if (ModDebugStates[modIndex].IsSet(1 << modBit))
		Cmd_PrintToConsole_Execute(PASS_COMMAND_ARGS);

	return true;
}

static bool Cmd_PrintD_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt8 modIndex = scriptObj->GetModIndex();
	UInt8 modBit = modIndex % 32;
	modIndex /= 32;

	if (ModDebugStates[modIndex].IsSet(1 << modBit))
	{
		ExpressionEvaluator eval(PASS_COMMAND_ARGS);
		if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->CanConvertTo(kTokenType_String))
		{
			const char* str = eval.Arg(0)->GetString();
			if (strlen(str) < 512)
				Console_Print(str);
			else
				Console_Print_Long(str);
		}
	}

	return true;
}

// this is very useful for debug output
bool Cmd_DBG_echo_Execute(COMMAND_ARGS)
{
	*result = 0;
	char buffer[kMaxMessageLength];

	if (ExtractFormatStringArgs(0, buffer, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_PrintToConsole.numParams))
	{
		if (strlen(buffer) < 512)
		{
			*result = 1;
			Console_Print(buffer);
		}
		else
			Console_Print_Long(buffer);

		_MESSAGE(buffer);
	}

	return true;
}

static bool Cmd_ToggleDebugText2_Execute(COMMAND_ARGS)
{
	InterfaceManager::ToggleDebugText();
	return true;
}

#endif

ParamInfo kParams_StringFormat[10] =
{
	{"string", kParamType_String, 0},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1}
};

ParamInfo kParams_StringFormatFile[11] =
{
	{"string", kParamType_String, 0},
	{"string", kParamType_String, 0},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1},
	{"float", kParamType_Float, 1}
};

static ParamInfo kParams_Message[21] =
{
	{"format string",	kParamType_String, 0},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
};

CommandInfo kCommandInfo_PrintToConsole =
{
	"PrintToConsole",
	"printc",
	0,
	"Print formatted string to console",
	0,
	21,
	kParams_Message,
	HANDLER(Cmd_PrintToConsole_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_PrintToFile =
{
	"PrintToFile",
	"printf",
	0,
	"Append formatted string to file",
	0,
	11,
	kParams_StringFormatFile,
	HANDLER(Cmd_PrintToFile_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_RunBatchScript[3] =
{
	{ "filename",	kParamType_String,	0	},
	{ "run on ref",	kParamType_Integer,	1	},
	{ "suppress output", kParamType_Integer, 1 },
};

CommandInfo kCommandInfo_RunBatchScript =
{
	"RunBatchScript",
	"",
	0,
	"Similar to the \'bat\' console command",
	0,
	3,
	kParams_RunBatchScript,
	HANDLER(Cmd_RunBatchScript_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(SetDebugMode,
			   toggles debug mode for all scripts belonging to a mod,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInt);

DEFINE_COMMAND(DebugPrint,
			   prints a formatted string to the console if debug mode is enabled for calling script,
			   0,
			   21,
			   kParams_Message);

static ParamInfo kParams_RunScriptLine[SIZEOF_FMT_STRING_PARAMS + 2] =
{
	FORMAT_STRING_PARAMS,
	{ "run on ref",	kParamType_Integer,	1	},
	{ "suppress output", kParamType_Integer, 1 },
};

DEFINE_COMMAND(RunScriptLine,
			   runs a line of script specified by a format string,
			   0,
			   SIZEOF_FMT_STRING_PARAMS + 2,
			   kParams_RunScriptLine);

DEFINE_COMMAND(DBG_echo, echo output to file and console, 0, 21, kParams_Message);

static ParamInfo kParams_OneOBSEString[] =
{
	{	"string",	kOBSEParamType_String,	0	},
};

CommandInfo kCommandInfo_PrintD =
{
	"PrintD",
	"",
	0,
	"prints a string expression to the console if debug mode is enabled for the calling script",
	0,
	1,
	kParams_OneOBSEString,
	HANDLER(Cmd_PrintD_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_CMD_ALT(ToggleDebugText2, TDT2, toggles debug text off without leaving the text frozen on the screen if called during gamemode, 0, NULL);