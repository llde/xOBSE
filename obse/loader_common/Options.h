#pragma once
#include <vector>

class Options
{
public:
	Options();
	~Options();

	bool	Read(LPSTR args);

	void	PrintUsage(void);
	std::vector<std::string> GetArgumentsList(LPSTR args);

	bool	m_launchCS;
	bool	m_loadOldblivion;
	DWORD	m_threadTimeout;

	bool	m_setPriority;
	DWORD	m_priority;

	bool	m_crcOnly;
	bool	m_noSync;
	bool	m_optionsOnly;
	bool	m_waitForClose;
	bool	m_oldInject;
	bool	m_ignorecrc;

	std::string	m_altEXE;
	std::string	m_altDLL;
};

extern Options g_options;
