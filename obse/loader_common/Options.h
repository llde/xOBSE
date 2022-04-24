#pragma once

class Options
{
public:
	Options();
	~Options();

	bool	Read(int argc, char ** argv);

	void	PrintUsage(void);

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
	bool	m_noConsole;

	std::string	m_altEXE;
	std::string	m_altDLL;

private:
	bool	Verify(void);
};

extern Options g_options;
