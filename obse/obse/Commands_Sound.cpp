#include "Commands_Sound.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "GameAPI.h"
#include "GameForms.h"

enum {
	kAtten_Min,
	kAtten_Max,
	kAtten_Static,

	kAtten_NONE
};

static UInt32 AttenCodeForString(const char* str)
{
	if (!_stricmp(str, "max"))
		return kAtten_Max;
	else if (!_stricmp(str, "min"))
		return kAtten_Min;
	else if (!_stricmp(str, "static"))
		return kAtten_Static;
	else
		return kAtten_NONE;
}

static bool Cmd_GetSoundAttenuation_Execute(COMMAND_ARGS)
{
	TESSound* sound = NULL;
	char whichStr[0x20] = { 0 };
	*result = -1.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &sound, whichStr) && sound) {
		UInt32 which = AttenCodeForString(whichStr);
		switch (which) {
			case kAtten_Min:
				*result = sound->minAttenuation * 5;
				break;
			case kAtten_Max:
				*result = sound->maxAttenuation * 100;
				break;
			case kAtten_Static:
				*result = sound->staticAttenuation / 100.0;
				break;
		}
	}

	if (IsConsoleMode()) {
		Console_Print("GetSoundAttenuation >> %.2f", *result);
	}

	return true;
}

static bool Cmd_SetSoundAttenuation_Execute(COMMAND_ARGS)
{
	TESSound* sound = NULL;
	char whichStr[0x20] = { 0 };
	float newAtten = 0.0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &sound, whichStr, &newAtten) && sound && newAtten >= 0.0) {
		UInt32 which = AttenCodeForString(whichStr);
		switch (which) {
			case kAtten_Min:
				newAtten /= 5;
				if (newAtten < 0x100) {
					sound->minAttenuation = newAtten;
					*result = 1.0;
				}
				break;
			case kAtten_Max:
				newAtten /= 100;
				if (newAtten <= 0x100) {
					sound->maxAttenuation = newAtten;
					*result = 1.0;
				}
				break;
			case kAtten_Static:
				if (newAtten <= 100) {	// CS limit
					sound->staticAttenuation = newAtten * 100;
					*result = 1.0;
				}
				break;
		}
	}

	if (IsConsoleMode()) {
		Console_Print("SetSoundAttenuation >> %.0f", *result);
	}

	return true;
}

#endif

static ParamInfo kParams_OneSound_OneString[2] =
{
	{ "sound", kParamType_Sound, 0 },
	{ "which", kParamType_String, 0 },
};

DEFINE_COMMAND(GetSoundAttenuation, returns the attenuation of a sound, 0, 2, kParams_OneSound_OneString);

static ParamInfo kParams_SetSoundAttenuation[3] =
{
	{ "sound", kParamType_Sound, 0 },
	{ "which", kParamType_String, 0 },
	{ "atten", kParamType_Float, 0 },
};

DEFINE_COMMAND(SetSoundAttenuation, sets the attenuation of a sound, 0, 3, kParams_SetSoundAttenuation);