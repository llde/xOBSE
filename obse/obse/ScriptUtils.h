#pragma once
#include <obse/ExpressionEvaluator.h>

/*
	Expressions are evaluated according to a set of rules stored in lookup tables. Each type of operand can
	be resolved to zero or more	"lower" types as defined by ConversionRule. Operators use OperationRules based
	on the types of each operand to	determine the result type. Types can be ambiguous at compile-time (array
	elements, command results) but always resolve to a concrete type at run-time. Rules are applied in the order
	they are defined until the first one matching the operands is encountered. At run-time this means the routines
	which perform the operations can know that the operands are of the expected type.
*/

struct Operator;
struct ScriptEventList;
class ExpressionEvaluator;
struct UserFunctionParam;
struct FunctionInfo;
struct FunctionContext;
class FunctionCaller;

#include "ScriptTokens.h"

bool IsCseLoaded();
bool DoesCseSupportCompilerWarnings();

extern ErrOutput g_ErrOut;

// these are used in ParamInfo to specify expected Token_Type of args to commands taking OBSE expressions as args
enum {
	kOBSEParamType_Number =		(1 << kTokenType_Number) | (1 << kTokenType_Ambiguous),
	kOBSEParamType_Boolean =	(1 << kTokenType_Boolean) | (1 << kTokenType_Ambiguous),
	kOBSEParamType_String =		(1 << kTokenType_String) | (1 << kTokenType_Ambiguous),
	kOBSEParamType_Form =		(1 << kTokenType_Form) | (1 << kTokenType_Ambiguous),
	kOBSEParamType_Array =		(1 << kTokenType_Array) | (1 << kTokenType_Ambiguous),
	kOBSEParamType_ArrayElement = 1 << (kTokenType_ArrayElement) | (1 << kTokenType_Ambiguous),
	kOBSEParamType_Slice =		1 << kTokenType_Slice,
	kOBSEParamType_Command =	1 << kTokenType_Command,
	kOBSEParamType_Variable =	1 << kTokenType_Variable,
	kOBSEParamType_NumericVar =	1 << kTokenType_NumericVar,
	kOBSEParamType_RefVar =		1 << kTokenType_RefVar,
	kOBSEParamType_StringVar =	1 << kTokenType_StringVar,
	kOBSEParamType_ArrayVar =	1 << kTokenType_ArrayVar,
	kOBSEParamType_ForEachContext = 1 << kTokenType_ForEachContext,

	kOBSEParamType_Collection = kOBSEParamType_Array | kOBSEParamType_String,
	kOBSEParamType_ArrayVarOrElement = kOBSEParamType_ArrayVar | kOBSEParamType_ArrayElement,
	kOBSEParamType_ArrayIndex = kOBSEParamType_String | kOBSEParamType_Number,
	kOBSEParamType_BasicType = kOBSEParamType_Array | kOBSEParamType_String | kOBSEParamType_Number | kOBSEParamType_Form,
	kOBSEParamType_NoTypeCheck = 0,

	kOBSEParamType_FormOrNumber = kOBSEParamType_Form | kOBSEParamType_Number,
	kOBSEParamType_StringOrNumber = kOBSEParamType_String | kOBSEParamType_Number,
	kOBSEParamType_Pair	=	1 << kTokenType_Pair,
};

// wraps a dynamic ParamInfo array
struct DynamicParamInfo
{
private:
	static const UInt32 kMaxParams = 10;

	ParamInfo	m_paramInfo[kMaxParams];
	UInt32		m_numParams;

public:
	DynamicParamInfo(std::vector<UserFunctionParam> &params);
	DynamicParamInfo() : m_numParams(0) { }

	ParamInfo* Params()	{	return m_paramInfo;	}
	UInt32 NumParams()	{ return m_numParams;	}
};

bool BasicTokenToElem(ScriptToken* token, ArrayElement& elem, ExpressionEvaluator* context);


struct CompilerMessages
{
	// NOTE: new messages MUST be added just before kMessageCode_Max
	// as the CSE plugin depends on the order of the ordinals
	enum MessageCode : UInt32			// varargs
	{
		kError_CantParse,
		kError_TooManyOperators,
		kError_TooManyOperands,
		kError_MismatchedBrackets,
		kError_InvalidOperands,			// string:operator
		kError_MismatchedQuotes,
		kError_InvalidDotSyntax,
		kError_CantFindVariable,		// string:varName
		kError_ExpectedStringVariable,
		kError_UnscriptedObject,		// string:objName
		kError_TooManyArgs,
		kError_RefRequired,				// string:commandName
		kError_MissingParam,			// string:paramName, int:paramIndex
		kError_UserFuncMissingArgList,	// string:userFunctionName
		kError_ExpectedUserFunction,
		kError_UserFunctionContainsMultipleBlocks,
		kError_UserFunctionVarsMustPrecedeDefinition,
		kError_UserFunctionParamsUndefined,
		kError_ExpectedStringLiteral,

		kWarning_UnquotedString,		// string:unquotedString
		kWarning_FunctionPointer,
		kWarning_DeprecatedCommand,		// string:commandName

		kMessageCode_Max
	};
private:
	static ErrOutput::Message s_Messages[];
public:
	static void Show(UInt32 messageCode, ScriptBuffer* scriptBuffer, ...);
};


class ExpressionParser
{
	enum { kMaxArgs = OBSE_EXPR_MAX_ARGS };

	ScriptBuffer		* m_scriptBuf;
	ScriptLineBuffer	* m_lineBuf;
	UInt32				m_len;
	Token_Type			m_argTypes[kMaxArgs];
	UInt8				m_numArgsParsed;

	char	Peek(UInt32 idx = -1) {
		if (idx == -1)	idx = m_lineBuf->lineOffset;
		return (idx < m_len) ? m_lineBuf->paramText[idx] : 0;
	}
	UInt32&	Offset()	{ return m_lineBuf->lineOffset; }
	const char * Text()	{ return m_lineBuf->paramText; }
	const char * CurText() { return Text() + Offset(); }

	Token_Type		Parse();
	Token_Type		ParseSubExpression(UInt32 exprLen);
	Operator *		ParseOperator(bool bExpectBinaryOperator, bool bConsumeIfFound = true);
	ScriptToken	*	ParseOperand(Operator* curOp = NULL);
	ScriptToken *	PeekOperand(UInt32& outReadLen);
	bool			ParseFunctionCall(CommandInfo* cmdInfo);
	Token_Type		PopOperator(std::stack<Operator*> & ops, std::stack<Token_Type> & operands);

	UInt32	MatchOpenBracket(Operator* openBracOp);
	std::string GetCurToken();
	Script::VariableInfo* LookupVariable(const char* varName, Script::RefVariable* refVar = NULL);

public:
	ExpressionParser(ScriptBuffer* scriptBuf, ScriptLineBuffer* lineBuf);
	~ExpressionParser();

	bool			ParseArgs(ParamInfo* params, UInt32 numParams, bool bUsesOBSEParamTypes = true);
	bool			ValidateArgType(UInt32 paramType, Token_Type argType, bool bIsOBSEParam);
	bool			ParseUserFunctionCall();
	bool			ParseUserFunctionDefinition();
	ScriptToken	*	ParseOperand(bool (* pred)(ScriptToken* operand));
	Token_Type		ArgType(UInt32 idx) { return idx < kMaxArgs ? m_argTypes[idx] : kTokenType_Invalid; }
};

void ShowRuntimeError(Script* script, const char* fmt, ...);
bool PrecompileScript(ScriptBuffer* buf);

// OBSE analogue for Cmd_Default_Parse, accepts expressions as args
bool Cmd_Expression_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf);

extern Operator s_operators[];


