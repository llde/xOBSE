#include "Utilities.h"

#ifdef OBLIVION
#include "GameAPI.h"
#include "Script.h"
#include "GameOSDepend.h"
#include <algorithm>
#include <DbgHelp.h>
void PrintItemType(TESForm * form)
{
	Console_Print("%s (%s)", GetFullName(form), GetObjectClassName(form));
}

const char GetSeparatorChar(Script* script)
{
	if (IsConsoleMode())
	{
		if (script && script->GetModIndex() != 0xFF)
			return '|';
		else
			return '@';
	}
	else
		return '|';
}

const char* GetSeparatorChars(Script* script)
{
	if (IsConsoleMode())
	{
		if (script && script->GetModIndex() != 0xFF)
			return "|";
		else
			return "@";
	}
	else
		return "|";
}

#endif

#pragma warning (push)
#pragma warning (disable : 4200)
struct RTTIType
{
	void	* typeInfo;
	UInt32	pad;
	char	name[0];
};

struct RTTILocator
{
	UInt32		sig, offset, cdOffset;
	RTTIType	* type;
};
#pragma warning (pop)

// use the RTTI information to return an object's class name
const char * GetObjectClassName(void * objBase)
{
	const char	* result = "<no rtti>";

	__try
	{
		void		** obj = (void **)objBase;
		RTTILocator	** vtbl = (RTTILocator **)obj[0];
		RTTILocator	* rtti = vtbl[-1];
		RTTIType	* type = rtti->type;

		// starts with ,?
		if((type->name[0] == '.') && (type->name[1] == '?'))
		{
			// is at most 100 chars long
			for(UInt32 i = 0; i < 100; i++)
			{
				if(type->name[i] == 0)
				{
					// remove the .?AV
					result = type->name + 4;
					break;
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// return the default
	}

	return result;
}

void DumpClass(void * theClassPtr, UInt32 nIntsToDump)
{
	_MESSAGE("DumpClass:");
	UInt32* basePtr = (UInt32*)theClassPtr;

	gLog.Indent();

	if (!theClassPtr) return;
	for (UInt32 ix = 0; ix < nIntsToDump; ix++ ) {
		UInt32* curPtr = basePtr+ix;
		const char* curPtrName = NULL;
		UInt32 otherPtr = 0;
		float otherFloat = 0.0;
		const char* otherPtrName = NULL;
		if (curPtr) {
			curPtrName = GetObjectClassName((void*)curPtr);

			__try
			{
				otherPtr = *curPtr;
				otherFloat = *(float*)(curPtr);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				//
			}

			if (otherPtr) {
				otherPtrName = GetObjectClassName((void*)otherPtr);
			}
		}

		_MESSAGE("%3d +%03X ptr: 0x%08X: %32s *ptr: 0x%08x | %f: %32s", ix, ix*4, curPtr, curPtrName, otherPtr, otherFloat, otherPtrName);
	}

	gLog.Outdent();
}

std::string GetOblivionDirectory(void)
{
	static std::string s_oblivionDirectory;

	if(s_oblivionDirectory.empty())
	{
		// can't determine how many bytes we'll need, hope it's not more than MAX_PATH
		char	oblivionPathBuf[MAX_PATH];
		UInt32	oblivionPathLength = GetModuleFileName(GetModuleHandle(NULL), oblivionPathBuf, sizeof(oblivionPathBuf));

		if(oblivionPathLength && (oblivionPathLength < sizeof(oblivionPathBuf)))
		{
			std::string	oblivionPath(oblivionPathBuf, oblivionPathLength);

			// truncate at last slash
			std::string::size_type	lastSlash = oblivionPath.rfind('\\');
			if(lastSlash != std::string::npos)	// if we don't find a slash something is VERY WRONG
			{
				s_oblivionDirectory = oblivionPath.substr(0, lastSlash + 1);

				_DMESSAGE("oblivion root = %s", s_oblivionDirectory.c_str());
			}
			else
			{
				_WARNING("no slash in oblivion path? (%s)", oblivionPath.c_str());
			}
		}
		else
		{
			_WARNING("couldn't find oblivion path (len = %d, err = %08X)", oblivionPathLength, GetLastError());
		}
	}

	return s_oblivionDirectory;
}

#if OBLIVION

const char*** g_KeyNames = (const char***)0xB39578;
const char*** g_ButtonNames = (const char***)0xB39554;

const char* GetDXDescription(UInt32 keycode)
{
	const char* keyName = "<no key>";

	if (keycode <= 220)
	{
		if (g_KeyNames[keycode])
			keyName = *(g_KeyNames[keycode]);
	}
	else if (255 <= keycode && keycode <= 263)
	{
		if (keycode == 255)
			keycode = 256;
		if (g_ButtonNames[keycode - 256])
			keyName = *(g_ButtonNames[keycode - 256]);
	}
	else if (keycode == 264)		//OB doesn't provide names for wheel up/down
		keyName = "WheelUp";
	else if (keycode == 265)
		keyName = "WheelDown";

	return keyName;
}

void Console_Print_Long(const std::string& str)
{
	UInt32 numLines = str.length() / 500;
	for (UInt32 i = 0; i < numLines; i++)
		Console_Print("%s ...", str.substr(i*500, 500).c_str());

	Console_Print("%s", str.substr(numLines*500, str.length() - numLines*500).c_str());
}

#endif

namespace MersenneTwister
{
/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void)
{
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6;
    return(a*67108864.0+b)*(1.0/9007199254740992.0);
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */

#undef N
#undef M
#undef MATRIX_A
#undef UPPER_MASK
#undef LOWER_MASK
};

Tokenizer::Tokenizer(const char* src, const char* delims)
	: m_offset(0), m_delims(delims), m_data(src)
{
	//
}

Tokenizer::~Tokenizer()
{
	//
}

UInt32 Tokenizer::NextToken(std::string& outStr)
{
	if (m_offset == m_data.length())
		return -1;

	size_t start = m_data.find_first_not_of(m_delims, m_offset);
	if (start != -1)
	{
		size_t end = m_data.find_first_of(m_delims, start);
		if (end == -1)
			end = m_data.length();

		m_offset = end;
		outStr = m_data.substr(start, end - start);
		return start;
	}

	return -1;
}

UInt32 Tokenizer::PrevToken(std::string& outStr)
{
	if (m_offset == 0)
		return -1;

	size_t searchStart = m_data.find_last_of(m_delims, m_offset - 1);
	if (searchStart == -1)
		return -1;

	size_t end = m_data.find_last_not_of(m_delims, searchStart);
	if (end == -1)
		return -1;

	size_t start = m_data.find_last_of(m_delims, end);	// okay if start == -1 here

	m_offset = end + 1;
	outStr = m_data.substr(start + 1, end - start);
	return start + 1;
}

#if 0

// max recursion depth is 10. We don't currently need anything close to that.
static UInt32 g_ThisStdCall_returnAddr[10] = { 0 };
static UInt32 g_ThisStdCall_stackAddr[10] = { 0 };

// Updated to allow recursive calls to ThisStdCall (still cheesy but meh)
// Issue came up with RunScriptLine "fileName" - LoadGame hook called ThisStdCall() again, overwriting the
// previously stored stack variables
__declspec(naked) UInt32 __cdecl ThisStdCall(UInt32 function, void * _this, ...)
{
	static UInt32 nestDepth = 0;
	static UInt32 _ebx = 0;				// ebx used as a working var, make sure original contents are preserved

	__asm
	{
		//// stack
		// return address <- esp
		// function
		// _this
		// args (unknown length)
		// ...

		//// target stack
		// return address <- esp
		// args (unknown length)

		pop	eax
		pop	edx
		pop	ecx

		mov _ebx, ebx
		mov ebx, nestDepth

		mov g_ThisStdCall_returnAddr[ebx * 4], eax
		mov g_ThisStdCall_stackAddr[ebx * 4], esp

		add ebx, 1
		mov nestDepth, ebx
		mov ebx, _ebx

		call edx

		mov _ebx, ebx
		mov ebx, nestDepth
		sub ebx, 1
		mov nestDepth, ebx

		mov esp, g_ThisStdCall_stackAddr[ebx * 4]

		push 0
		push 0
		push g_ThisStdCall_returnAddr[ebx * 4]

		mov ebx, _ebx
		retn
	}
}

#endif

bool ci_equal(char ch1, char ch2)
{
	return tolower((unsigned char)ch1) == tolower((unsigned char)ch2);
}

bool ci_less(const char* lh, const char* rh)
{
	ASSERT(lh && rh);
	while (*lh && *rh) {
		char l = toupper(*lh);
		char r = toupper(*rh);
		if (l < r) {
			return true;
		}
		else if (l > r) {
			return false;
		}
		lh++;
		rh++;
	}

	return toupper(*lh) < toupper(*rh);
}

void MakeUpper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void MakeLower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
}

#if OBLIVION

char* CopyCString(const char* src)
{
	UInt32 size = src ? strlen(src) : 0;
	char* result = (char*)FormHeap_Allocate(size+1);
	result[size] = 0;
	if (size) {
		strcpy_s(result, size+1, src);
	}

	return result;
}

#endif

#pragma warning(push)
#pragma warning(disable: 4996)	// warning about std::transform()

void MakeUpper(char* str)
{
	if (str) {
		UInt32 len = strlen(str);
		std::transform(str, str + len, str, toupper);
	}
}

void MakeLower(char* str)
{
	if (str) {
		UInt32 len = strlen(str);
		std::transform(str, str + len, str, tolower);
	}
}

#pragma warning(pop)

// ErrOutput
ErrOutput::ErrOutput(ErrorCallbackT errorFunc, WarningCallbackT warningFunc)
{
	ShowWarning = warningFunc;
	ShowError = errorFunc;
}

void ErrOutput::vShow(ErrOutput::Message& msg, void* userData, va_list args)
{
	if (msg.CanDisable() && msg.IsDisabled())
		return;

	char msgText[0x1000];
	vsprintf_s(msgText, sizeof(msgText), msg.fmt.c_str(), args);

	if (msg.IsTreatAsWarning())
	{
		bool disableWarning = ShowWarning(msgText, userData, msg.CanDisable());
		if (msg.CanDisable() && disableWarning)
			msg.SetDisabled();
	}
	else
		ShowError(msgText, userData);
}

void ErrOutput::Show(ErrOutput::Message msg, void* userData, ...)
{
	va_list args;
	va_start(args, userData);

	vShow(msg, userData, args);

	va_end(args);
}

void ErrOutput::Show(const char* msg, void* userData, ...)
{
	va_list args;
	va_start(args, userData);

	vShow(msg, userData, args);

	va_end(args);

}

void ErrOutput::vShow(const char* msg, void* userData, va_list args)
{
	Message tempMsg (msg);
	vShow(tempMsg, userData, args);
}

LPTOP_LEVEL_EXCEPTION_FILTER g_OriginalTopLevelExceptionFilter = NULL;

bool CreateExceptionMiniDump( _EXCEPTION_POINTERS *ExceptionInfo )
{
	HANDLE DumpFile = CreateFile("OBSECrashDump.dmp",
								GENERIC_READ|GENERIC_WRITE,
								0,
								NULL,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

	if (DumpFile == INVALID_HANDLE_VALUE)
		return false;

	MINIDUMP_EXCEPTION_INFORMATION mdei;

	mdei.ThreadId           = GetCurrentThreadId();
	mdei.ExceptionPointers  = ExceptionInfo;
	mdei.ClientPointers     = FALSE;

	MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpNormal|
							MiniDumpWithIndirectlyReferencedMemory|
							MiniDumpScanMemory|
							MiniDumpWithThreadInfo|
							MiniDumpWithProcessThreadData|
							MiniDumpWithUnloadedModules|
					//		MiniDumpWithDataSegs|
							MiniDumpWithFullMemoryInfo|
							MiniDumpWithHandleData);
#ifdef OBSE_CORE
	BOOL rv = MiniDumpWriteDump(GetCurrentProcess(),
								GetCurrentProcessId(),
								DumpFile,
								mdt, (ExceptionInfo != 0) ? &mdei : 0, 0, 0 );
	if( !rv )
		return false;
#endif

	CloseHandle(DumpFile);
	return true;
}

LONG WINAPI OBSEUnhandledExceptionFilter( __in struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	CreateExceptionMiniDump(ExceptionInfo);

#ifdef OBLIVION
#ifdef OBSE_CORE
	ShowWindow((*g_osGlobals)->window, SW_MINIMIZE);
#endif
	MessageBox(NULL, "Oblivion has crashed! A minidump has been created in the game directory.", "OBSE", MB_TASKMODAL|MB_SETFOREGROUND|MB_ICONERROR|MB_OK);
#else
	MessageBox(NULL, "The Construction Set has crashed! A minidump has been created in the game directory.", "OBSE", MB_TASKMODAL|MB_SETFOREGROUND|MB_ICONERROR|MB_OK);
#endif

	if (g_OriginalTopLevelExceptionFilter)
		return g_OriginalTopLevelExceptionFilter(ExceptionInfo);
	else
		return EXCEPTION_EXECUTE_HANDLER;
}
