#include "Options.h"
#include <vector>
#include <iostream>
#include <string>
#include <sstream> 

#include <obse/loader_common/EXEChecksum.h>

Options g_options;

Options::Options()
:m_launchCS(false)
,m_loadOldblivion(false)
,m_threadTimeout(1000 * 30)
,m_setPriority(false)
,m_priority(0)
,m_crcOnly(false)
,m_noSync(false)
,m_optionsOnly(false)
,m_waitForClose(false)
,m_oldInject(false)
{
	//
}

Options::~Options()
{
	//
}

// disable "switch statement contains 'default' but no 'case' labels"
#pragma warning (push)
#pragma warning (disable : 4065)

std::vector<std::string> Options::GetArgumentsList(LPSTR args) {
	std::vector<std::string> tokens;
	std::string orig = args;
	std::stringstream   mySstream(orig);
	std::string         temp;

	while (getline(mySstream, temp, ' ')) {
		tokens.push_back(temp);
	}

	return tokens;
}

bool Options::Read(LPSTR args)
{
	auto argl = GetArgumentsList(args);
	int	freeArgCount = 0;
	bool alt_dll = false;
	bool alt_exe = false;

	for (std::string&  arg : argl){
		if (alt_dll || alt_exe) {
			if (arg.starts_with('-')) {
				PrintError("Expected alternative DLL path got another option");
				return false;
			}
			if (alt_dll) {
				m_altDLL = arg;
				alt_dll = false;
			}
			if (alt_exe) {
				m_altEXE = arg;
				alt_exe = false;
			}
		}
		else if (arg == "-editor") { m_launchCS = true; }
		else if (arg == "-h" || arg == "--help") { m_optionsOnly = true; }
		else if (arg == "-altdll") { alt_dll = true; }
		else if (arg == "-altexe") { alt_exe = true; }
		else if (arg == "-old") { m_loadOldblivion = true; }
		else if (arg == "-oldinject") { m_oldInject = true; }
		else if (arg == "-notimeout") { m_threadTimeout = INFINITE; }
		else {
			PrintError("Unsupported option %s", arg.c_str());
			return false;
		}
	}
	

	return true;
}

#pragma warning (pop)

void Options::PrintUsage(void)
{
	gLog.SetPrintLevel(IDebugLog::kLevel_VerboseMessage);

	_MESSAGE("usage: obse_loader [options]");
	_MESSAGE("");
	_MESSAGE("options:");
	_MESSAGE("  -h, -help - print this options list");
	_MESSAGE("  -editor - launch the construction set");
	_MESSAGE("  -old - load oldblivion");
//	_MESSAGE("  -priority <level> - set the launched program\'s priority");
//	_MESSAGE("    above_normal");
//	_MESSAGE("    below_normal");
//	_MESSAGE("    high");
//	_MESSAGE("    idle");
//	_MESSAGE("    normal");
//	_MESSAGE("    realtime");
	_MESSAGE("  -altexe <path> - set alternate exe path");
	_MESSAGE("  -altdll <path> - set alternate dll path");
	_MESSAGE("  -notimeout - wait forever for oblivion to launch");
	_MESSAGE("               this overrides the default five second timeout");
	_MESSAGE("  -crconly - just print the crc of the EXE, don't launch anything");
//	_MESSAGE("  -nosync - WARNING: USING THIS OPTION MAY CAUSE BUGS AND OTHER RANDOM BEHAVIOR");
//	_MESSAGE("            disable synchronization when injecting the dll");
//	_MESSAGE("            this is only intended as a compatibility hack for WINE");
//	_MESSAGE("  -waitforclose - wait for the launched program to close");
//	_MESSAGE("                  designed for use with AlacrityPC and similar");
	_MESSAGE("  -oldinject - use the original injection method for the editor");
}

