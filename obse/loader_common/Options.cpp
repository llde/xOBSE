#include "Options.h"

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

bool Options::Read(int argc, char ** argv)
{
	if(argc >= 1)
	{
		// remove app name
		argc--;
		argv++;

		int	freeArgCount = 0;

		while(argc > 0)
		{
			char	* arg = *argv++;
			argc--;

			if(arg[0] == '-')
			{
				// switch
				arg++;

				if(!_stricmp(arg, "editor"))
				{
					m_launchCS = true;
				}
				else if(!_stricmp(arg, "old"))
				{
					m_loadOldblivion = true;
				}
				else if(!_stricmp(arg, "priority"))
				{
					if(argc >= 1)
					{
						arg = *argv++;
						argc--;

						m_setPriority = true;

						if(!_stricmp(arg, "above_normal"))
						{
							m_priority = ABOVE_NORMAL_PRIORITY_CLASS;
						}
						else if(!_stricmp(arg, "below_normal"))
						{
							m_priority = BELOW_NORMAL_PRIORITY_CLASS;
						}
						else if(!_stricmp(arg, "high"))
						{
							m_priority = HIGH_PRIORITY_CLASS;
						}
						else if(!_stricmp(arg, "idle"))
						{
							m_priority = IDLE_PRIORITY_CLASS;
						}
						else if(!_stricmp(arg, "normal"))
						{
							m_priority = NORMAL_PRIORITY_CLASS;
						}
						else if(!_stricmp(arg, "realtime"))
						{
							m_priority = REALTIME_PRIORITY_CLASS;
						}
						else
						{
							m_setPriority = false;

							_ERROR("couldn't read priority argument (%s)", arg);
							return false;
						}
					}
					else
					{
						_ERROR("priority not specified");
						return false;
					}
				}
				else if(!_stricmp(arg, "altexe"))
				{
					if(argc >= 1)
					{
						m_altEXE = *argv++;
						argc--;
					}
					else
					{
						_ERROR("exe path not specified");
						return false;
					}
				}
				else if(!_stricmp(arg, "altdll"))
				{
					if(argc >= 1)
					{
						m_altDLL = *argv++;
						argc--;
					}
					else
					{
						_ERROR("dll path not specified");
						return false;
					}
				}
				else if(!_stricmp(arg, "notimeout"))
				{
					m_threadTimeout = INFINITE;
				}
				else if(!_stricmp(arg, "crconly"))
				{
					m_crcOnly = true;
				}
				else if(!_stricmp(arg, "nosync"))
				{
					// ### only intended for use on WINE
					m_noSync = true;
				}
				else if(!_stricmp(arg, "h") || !_stricmp(arg, "help"))
				{
					m_optionsOnly = true;
				}
				else if(!_stricmp(arg, "waitforclose"))
				{
					m_waitForClose = true;
				}
				else if(!_stricmp(arg, "oldinject"))
				{
					m_oldInject = true;
				}
				else if(!_stricmp(arg, "noconsole"))
				{
					m_noConsole = true;
				}
				else
				{
					_ERROR("unknown switch (%s)", arg);
					return false;
				}
			}
			else
			{
				// free arg

				switch(freeArgCount)
				{
					default:
						_ERROR("too many free args (%s)", arg);
						return false;
				}
			}
		}
	}

	return Verify();
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
	_MESSAGE("  -priority <level> - set the launched program\'s priority");
	_MESSAGE("    above_normal");
	_MESSAGE("    below_normal");
	_MESSAGE("    high");
	_MESSAGE("    idle");
	_MESSAGE("    normal");
	_MESSAGE("    realtime");
	_MESSAGE("  -altexe <path> - set alternate exe path");
	_MESSAGE("  -altdll <path> - set alternate dll path");
	_MESSAGE("  -notimeout - wait forever for oblivion to launch");
	_MESSAGE("               this overrides the default five second timeout");
	_MESSAGE("  -crconly - just print the crc of the EXE, don't launch anything");
	_MESSAGE("  -nosync - WARNING: USING THIS OPTION MAY CAUSE BUGS AND OTHER RANDOM BEHAVIOR");
	_MESSAGE("            disable synchronization when injecting the dll");
	_MESSAGE("            this is only intended as a compatibility hack for WINE");
	_MESSAGE("  -waitforclose - wait for the launched program to close");
	_MESSAGE("                  designed for use with AlacrityPC and similar");
	_MESSAGE("  -oldinject - use the original injection method for the editor");
	_MESSAGE("  -noconsole - do not display a terminal");
}

bool Options::Verify(void)
{
	// nothing to verify currently

	return true;
}
