#include "Utilities.h"
#include "Settings.h"

UInt8 installCrashdump;
UInt8 warningUnquotedString;
UInt8 warningUDFRefVar;
UInt8 warningDeprecatedCmd;
bool FreeRef;
bool NoisyTestExpr;
bool PreventCrashOnMapMarkerLoadSave;

bool InitializeSettings() {
	std::string	runtimePath = GetOblivionDirectory();
	//TODO create non existant INI
	if (!runtimePath.empty())
	{
		s_configPath = runtimePath;
		s_configPath += OBSEINI;

		_MESSAGE("config path = %s", s_configPath.c_str());
	}
	installCrashdump = GetPrivateProfileInt(INI_SECTION_RUNTIME, INI_RUNTIME_CRASHDUMP, 0, s_configPath.c_str());
	warningUDFRefVar = GetPrivateProfileInt(INI_SECTION_COMPILER, INI_COMPILER_WARNFUNCTPTR, 1, s_configPath.c_str());
	warningUnquotedString = GetPrivateProfileInt(INI_SECTION_COMPILER, INI_COMPILER_WARNUNQUOTEDSTRING, 1, s_configPath.c_str());
	warningDeprecatedCmd = GetPrivateProfileInt(INI_SECTION_COMPILER, INI_COMPILER_WARNDEPRECATEDCMD, 1, s_configPath.c_str());
	FreeRef = GetPrivateProfileInt(INI_SECTION_RUNTIME, "bDeallocateReferences", 0, s_configPath.c_str());
	NoisyTestExpr = GetPrivateProfileInt(INI_SECTION_RUNTIME, "bTestExprComplainsOnError", 0, s_configPath.c_str());
	PreventCrashOnMapMarkerLoadSave = GetPrivateProfileInt(INI_SECTION_RUNTIME, "bPreventCrashOnMapMarkerLoad", 1, s_configPath.c_str());

    return true;
}
