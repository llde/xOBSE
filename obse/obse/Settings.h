#pragma once

#define OBSEINI "Data\\OBSE\\obse.ini"
#define INI_SECTION_RUNTIME		"Runtime"
#define INI_RUNTIME_CRASHDUMP	"bCreateCrashDump"
#define INI_SECTION_COMPILER		"Compiler"
#define INI_COMPILER_WARNUNQUOTEDSTRING		"bWarningUnquotedString"
#define INI_COMPILER_WARNFUNCTPTR			"bWarningUDFRefVar"


static std::string s_configPath;

extern UInt8 installCrashdump;
extern UInt8 warningUnquotedString;
extern UInt8 warningUDFRefVar;
extern bool FreeRef;

bool InitializeSettings();