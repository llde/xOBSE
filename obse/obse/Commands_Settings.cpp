
#if OBLIVION

static bool s_bCanEditInitIni = false;









#endif

CommandInfo kCommandInfo_ConsoleEnableIniEdit
{
	"EnableIniEdit",
	"editini",
	0,
	"console command to backup ini and enable edits if 1 and disable ini edits if 0",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_ConsoleEnableInitEdit_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

extern CommandInfo kCommandInfo_IsIniEditEnabled
{
	"IsIniEditEnabled",
	"ceini",	// can edit ini
	0,
	"returns a ref to the type of the specified equipment slot",
	0,
	0,
	NULL,
	HANDLER(Cmd_CanEditIni_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


extern CommandInfo kCommandInfo_SaveIni
{
	"SaveIni",
	"svini",	// can edit ini
	0,
	"saves the ini file if ini editing is enabled",
	0,
	0,
	NULL,
	HANDLER(Cmd_SaveIni_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};




extern CommandInfo kCommandInfo_RefreshIni
{
	"RefreshIni",
	"rfshini",	// can edit ini
	0,
	"refrshes the ini from disk if ini editing enabled",
	0,
	0,
	NULL,
	HANDLER(Cmd_RefreshIni_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

extern CommandInfo kCommandInfo_RestoreIni
{
	"RestoreInit",
	"rtrini",	// can edit ini
	0,
	"restores the ini from the backup if ini editing is enabled",
	0,
	0,
	NULL,
	HANDLER(Cmd_CanEditIni_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

