#pragma once
#include <obse/GameForms.h>
#include <obse/ScriptTokens.h>

#define OBSE_EXPR_MAX_ARGS 20		// max # of args we'll accept to a commmand

#ifdef OBLIVION
class ExpressionEvaluator
{
	enum { kMaxArgs = OBSE_EXPR_MAX_ARGS };

	enum {
		kFlag_SuppressErrorMessages = 1 << 0,
		kFlag_ErrorOccurred = 1 << 1,
		kFlag_StackTraceOnError = 1 << 2,
	};

	Bitfield<UInt32>	 m_flags;
	UInt8* m_scriptData;
	UInt32* m_opcodeOffsetPtr;
	double* m_result;
	TESObjectREFR* m_thisObj;
	TESObjectREFR* m_containingObj;
	UInt8* m_data;
	ScriptToken* m_args[kMaxArgs];
	ParamInfo* m_params;
	UInt8				m_numArgsExtracted;
	CommandReturnType	m_expectedReturnType;
	UInt16				m_baseOffset;
	ExpressionEvaluator* m_parent;

	CommandReturnType GetExpectedReturnType() { CommandReturnType type = m_expectedReturnType; m_expectedReturnType = kRetnType_Default; return type; }

	void PushOnStack();
	void PopFromStack();
public:
	static bool	Active();

	ExpressionEvaluator(COMMAND_ARGS);
	~ExpressionEvaluator();

	Script* script;
	ScriptEventList* eventList;

	void			Error(const char* fmt, ...);
	void			Error(const char* fmt, ScriptToken* tok, ...);
	bool			HasErrors() { return m_flags.IsSet(kFlag_ErrorOccurred); }

	// extract args compiled by ExpressionParser
	bool			ExtractArgs();

	// extract args to function which normally uses Cmd_Default_Parse but has been compiled instead by ExpressionParser
	// bConvertTESForms will be true if invoked from ExtractArgs(), false if from ExtractArgsEx()
	bool			ExtractDefaultArgs(va_list varArgs, bool bConvertTESForms);

	// convert an extracted argument to type expected by ExtractArgs/Ex() and store in varArgs
	bool			ConvertDefaultArg(ScriptToken* arg, ParamInfo* info, bool bConvertTESForms, va_list& varArgs);

	// extract formatted string args compiled with compiler override
	bool ExtractFormatStringArgs(va_list varArgs, UInt32 fmtStringPos, char* fmtStringOut, UInt32 maxParams);

	ScriptToken* Evaluate();			// evaluates a single argument/token

	ScriptToken* Arg(UInt32 idx) { return idx < kMaxArgs ? m_args[idx] : NULL; }
	UInt8			NumArgs() { return m_numArgsExtracted; }
	void			SetParams(ParamInfo* newParams) { m_params = newParams; }
	void			ExpectReturnType(CommandReturnType type) { m_expectedReturnType = type; }
	void			ToggleErrorSuppression(bool bSuppress);
	void			PrintStackTrace();

	TESObjectREFR* ThisObj() { return m_thisObj; }
	TESObjectREFR* ContainingObj() { return m_containingObj; }

	UInt8		ReadByte();
	UInt16		Read16();
	double		ReadFloat();
	std::string	ReadString();
	SInt8		ReadSignedByte();
	SInt16		ReadSigned16();
	UInt32		Read32();
	SInt32		ReadSigned32();
};

class OverriddenScriptFormatStringArgs : public FormatStringArgs
{
public:
	OverriddenScriptFormatStringArgs(ExpressionEvaluator* eval, UInt32 fmtStringPos) : m_eval(eval), m_curArgIndex(fmtStringPos + 1) {
		m_fmtString = m_eval->Arg(fmtStringPos)->GetString();
	}

	virtual bool Arg(argType asType, void* outResult) {
		ScriptToken* arg = m_eval->Arg(m_curArgIndex);
		if (arg) {
			switch (asType) {
			case kArgType_Float:
				*((double*)outResult) = arg->GetNumber();
				break;
			case kArgType_Form:
				*((TESForm**)outResult) = arg->GetTESForm();
				break;
			default:
				return false;
			}

			m_curArgIndex += 1;
			return true;
		}
		else {
			return false;
		}
	}

	virtual bool SkipArgs(UInt32 numToSkip) { m_curArgIndex += numToSkip; return m_curArgIndex <= m_eval->NumArgs(); }
	virtual bool HasMoreArgs() { return m_curArgIndex < m_eval->NumArgs(); }
	virtual std::string GetFormatString() { return m_fmtString; }

	UInt32 GetCurArgIndex() const { return m_curArgIndex; }
private:
	ExpressionEvaluator* m_eval;
	UInt32					m_curArgIndex;
	const char* m_fmtString;
};

#endif