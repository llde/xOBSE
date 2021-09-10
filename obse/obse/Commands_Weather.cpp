#include "obse/Commands_Weather.h"
#include "obse/ParamInfos.h"
#include "Script.h"


#if OBLIVION
#include "obse/GameObjects.h"
#include "obse/GameForms.h"
#include "obse/Utilities.h"

static bool Cmd_GetWeatherInfo_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESWeather* weather = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &weather))
		return true;
	

	TESForm* form = LookupFormByID(0x473f9);
	TESClimate* climate = (TESClimate*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESClimate, 0);

	TESObjectCELL* playerCell = (*g_thePlayer)->parentCell;

	int x = 0;

	return true;
}

static bool Cmd_GetCurrentWeatherID_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	Sky* sky = Sky::GetSingleton();
	if (!sky) return true;

	TESWeather* weather = sky->firstWeather;

	if (weather) {
		*refResult = weather->refID;
	}

	return true;
}

static bool Cmd_GetCurrentClimateID_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	Sky* sky = Sky::GetSingleton();
	if (!sky) return true;

	TESClimate* climate = sky->firstClimate;

	if (climate) {
		*refResult = climate->refID;
	}

	return true;
}

static bool Cmd_SetCurrentClimate_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;
	
	TESForm* form = NULL;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form);
	TESClimate* climate = (TESClimate*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESClimate, 0);
	if (!climate) return true;

	Sky* sky = Sky::GetSingleton();
	if (!sky) return true;
	
	TESClimate* oldClimate = sky->firstClimate;
	if (oldClimate) {
		*refResult = oldClimate->refID;
	}
	sky->firstClimate = climate;

	sky->RefreshClimate(sky->firstClimate, 1);
	return true;
}

static bool Cmd_SetWorldspaceClimate_Execute(COMMAND_ARGS)
{
	UInt32			* refResult = (UInt32 *)result;
	TESWorldSpace	* worldspace = NULL;
	TESClimate		* climate = NULL;

	*result = 0;

	if(!ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &worldspace, &climate)) return true;
	if(!worldspace) return true;

	TESWorldSpace	* firstWorldspace = worldspace;
	UInt32			loopCheck = 0;

	while(worldspace->parentWorldspace)
	{
		worldspace = worldspace->parentWorldspace;

		// paranoia because this might loop forever
		loopCheck++;
		if(loopCheck >= 100)
			break;
	}

	if(worldspace->climate)
		*refResult = worldspace->climate->refID;

	worldspace->climate = climate;

	return true;
}

static bool Cmd_RefreshCurrentClimate_Execute(COMMAND_ARGS)
{
	Sky* sky = Sky::GetSingleton();
	if (!sky) return true;
	sky->RefreshClimate(sky->firstClimate, 1);
	return true;
}

enum {
	kClimate_SunriseBegin,
	kClimate_SunriseEnd,
	kClimate_SunsetBegin,
	kClimate_SunsetEnd,
	kClimate_MoonPhaseLength,
	kClimate_HasMasser,
	kClimate_HasSecunda,
	kClimate_Volatility,
};

static bool GetClimateValue_Execute(COMMAND_ARGS, UInt32 whichVal)
{
	*result = 0;
	TESForm* form = NULL;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form);
	TESClimate* climate = (TESClimate*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESClimate, 0);
	if (!climate) return true;

	switch (whichVal) {
		case kClimate_SunriseBegin:
			*result = climate->sunriseBegin;
			break;
		case kClimate_SunriseEnd:
			*result = climate->sunriseEnd;
			break;
		case kClimate_SunsetBegin:
			*result = climate->sunsetBegin;
			break;
		case kClimate_SunsetEnd:
			*result = climate->sunsetEnd;
			break;
		case kClimate_MoonPhaseLength:
			*result = climate->GetPhaseLength();
			break;
		case kClimate_HasMasser:
			*result = climate->HasMasser() ? 1 : 0;
			break;
		case kClimate_HasSecunda:
			*result = climate->HasSecunda() ? 1 : 0;
			break;
	}
	return true;
}

static bool Cmd_GetClimateSunriseBegin_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunriseBegin);
}

static bool Cmd_GetClimateSunriseEnd_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunriseEnd);
}

static bool Cmd_GetClimateSunsetBegin_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunsetBegin);
}

static bool Cmd_GetClimateSunsetEnd_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunsetEnd);
}

static bool Cmd_GetClimateMoonPhaseLength_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_MoonPhaseLength);
}

static bool Cmd_GetClimateHasMasser_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_HasMasser);
}

static bool Cmd_GetClimateHasSecunda_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_HasSecunda);
}

static bool Cmd_GetClimateVolatility_Execute(COMMAND_ARGS)
{
	return GetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_Volatility);
}

static bool SetClimateValue_Execute(COMMAND_ARGS, UInt32 whichVal)
{
	*result = 0;
	UInt32 intVal = 0;
	TESForm* form = NULL;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &intVal, &form);
	TESClimate* climate = (TESClimate*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESClimate, 0);
	if (!climate) return true;

	UInt8 nuVal = (UInt8)intVal;

	switch (whichVal) {
		case kClimate_SunriseBegin:
			climate->SetSunriseBegin(nuVal);
			break;
		case kClimate_SunriseEnd:
			climate->SetSunriseEnd(nuVal);
			break;
		case kClimate_SunsetBegin:
			climate->SetSunsetBegin(nuVal);
			break;
		case kClimate_SunsetEnd:
			climate->SetSunsetEnd(nuVal);
			break;
		case kClimate_MoonPhaseLength:
			climate->SetPhaseLength(nuVal);
			break;
		case kClimate_HasMasser:
			climate->SetHasMasser(nuVal != 0);
			break;
		case kClimate_HasSecunda:
			climate->SetHasSecunda(nuVal != 0);
			break;
		case kClimate_Volatility:
			climate->volatility = nuVal;
			break;
	}
	return true;
}

static bool Cmd_SetClimateSunriseBegin_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunriseBegin);
}

static bool Cmd_SetClimateSunriseEnd_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunriseEnd);
}

static bool Cmd_SetClimateSunsetBegin_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunsetBegin);
}

static bool Cmd_SetClimateSunsetEnd_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_SunsetEnd);
}

static bool Cmd_SetClimateMoonPhaseLength_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_MoonPhaseLength);
}

static bool Cmd_SetClimateHasMasser_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_HasMasser);
}

static bool Cmd_SetClimateHasSecunda_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_HasSecunda);
}

static bool Cmd_SetClimateVolatility_Execute(COMMAND_ARGS)
{
	return SetClimateValue_Execute(PASS_COMMAND_ARGS, kClimate_Volatility);
}

enum {
	kWeather_WindSpeed,
	kWeather_CloudSpeedLower,
	kWeather_CloudSpeedUpper,
	kWeather_TransDelta,
	kWeather_SunGlare,
	kWeather_SunDamage,
	kWeather_FogDayNear,
	kWeather_FogDayFar,
	kWeather_FogNightNear,
	kWeather_FogNightFar,
	kWeather_PrecipType,
	kWeather_LightningFrequency,
	kWeather_LightningColorRed,
	kWeather_LightningColorGreen,
	kWeather_LightningColorBlue,
};

static bool GetWeatherValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	TESWeather* weather = NULL;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &weather);
	if (!weather) return true;
	
	switch(whichValue) {
			case kWeather_WindSpeed:
				{
					*result = (weather->windSpeed/255.0);
					break;
				}

			case kWeather_CloudSpeedLower:
				{
					*result = (weather->cloudSpeedLower/255.0);
					break;
				}
			case kWeather_CloudSpeedUpper:
				{
					*result = (weather->cloudSpeedUpper/255.0);
					break;
				}
			case kWeather_TransDelta:
				{
					*result = (weather->transDelta/255.0);
					break;
				}

			case kWeather_SunGlare: 
				{
					*result = (weather->sunGlare/255.0);
					break;
				}
			case kWeather_SunDamage:
				{
					*result = (weather->sunDamage/255.0);
					break;
				}
			case kWeather_FogDayNear:
				{
					*result = weather->fogDay.nearFog;
					break;
				}
			case kWeather_FogDayFar:
				{
					*result = weather->fogDay.farFog;
					break;
				}
			case kWeather_FogNightNear:
				{
					*result = weather->fogNight.nearFog;
					break;
				}
			case kWeather_FogNightFar:
				{
					*result = weather->fogNight.farFog;
					break;
				}

			case kWeather_PrecipType:
				{
					*result = weather->precipType;
					break;
				}

			case kWeather_LightningFrequency:
				{
					*result = weather->lightningFrequency;
					break;
				}
			
			case kWeather_LightningColorRed:
				{
					*result = weather->lightningColor.r;
					break;
				}
			case kWeather_LightningColorGreen:
				{
					*result = weather->lightningColor.g;
					break;
				}

			case kWeather_LightningColorBlue:
				{
					*result = weather->lightningColor.b;
					break;
				}

			default:
				break;
	}
	return true;
}

static bool Cmd_GetWeatherWindSpeed_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_WindSpeed);
}

static bool Cmd_GetWeatherCloudSpeedLower_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_CloudSpeedLower);
}

static bool Cmd_GetWeatherCloudSpeedUpper_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_CloudSpeedUpper);
}

static bool Cmd_GetWeatherTransDelta_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_TransDelta);
}

static bool Cmd_GetWeatherSunGlare_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_SunGlare);
}

static bool Cmd_GetWeatherSunDamage_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_SunDamage);
}

static bool Cmd_GetWeatherFogDayNear_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogDayNear);
}

static bool Cmd_GetWeatherFogDayFar_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogDayFar);
}

static bool Cmd_GetWeatherFogNightNear_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogNightNear);
}

static bool Cmd_GetWeatherFogNightFar_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogNightFar);
}

static bool Cmd_GetWeatherPrecipitationType_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_PrecipType);
}

static bool Cmd_GetWeatherLightningFrequency_Execute(COMMAND_ARGS)
{
	return GetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_LightningFrequency);
}

static bool Cmd_GetWeatherHDRValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 whichValue;
	TESWeather* weather = NULL;
	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &whichValue, &weather);
	if (!weather) return true;
	if (whichValue > TESWeather::eHDR_Last) return true;

	*result = weather->GetHDRValue(whichValue);
	return true;
}

static bool Cmd_SetWeatherHDRValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	float floatVal = 0.0;
	UInt32 whichValue = 0;
	TESWeather* weather = NULL;
	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &floatVal, &whichValue, &weather);
	if (!weather) return true;
	if (whichValue > TESWeather::eHDR_Last) return true;

	*result = weather->SetHDRValue(whichValue, floatVal);
	return true;
}

static bool Cmd_GetWeatherColor_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 rgb = 0; // red = 0, green = 1, blue = 0
	UInt32 whichColor = 0;
	TESWeather* weather = NULL;
	UInt32 time = 0; // sunrise = 0, day = 1, sunset = 2, night = 3 

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &rgb, &whichColor, &weather, &time);
	if (!weather) return true;
	if (whichColor > TESWeather::eColor_Last) return true;
	if (time > TESWeather::eTime_Night) return true;
	if (rgb > 2) return true;

	TESWeather::RGBA& rgba = weather->GetColor(whichColor, time);
	if (rgb == 0) *result = rgba.r;
	else if (rgb == 1) *result = rgba.g;
	else *result = rgba.b;
	return true;
}

static bool Cmd_SetWeatherColor_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 red = 0;
	UInt32 green = 0;
	UInt32 blue = 0;
	UInt32 whichColor = 0;
	TESWeather* weather = NULL;
	UInt32 time = 0;
	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &red, &green, &blue, &whichColor, &weather, &time);
	if (!weather) return true;
	if (whichColor > TESWeather::eColor_Last) return true;
	if (time > TESWeather::eTime_Night) return true;

	if (red > 255) red = 255;
	if (green > 255) green = 255;
	if (blue > 255) blue = 255;

	TESWeather::RGBA& rgba = weather->GetColor(whichColor, time);
	rgba.Set(red, green, blue);
	return true;
}

static bool SetWeatherValue_Execute(COMMAND_ARGS, UInt32 which)
{
	*result = 0;

	TESWeather* weather = NULL;
	UInt32 intVal = 0;
	float floatVal = 0.0;
	
	switch(which) {
		// extract float and convert to int <= 255
		case kWeather_WindSpeed:
		case kWeather_CloudSpeedLower:
		case kWeather_CloudSpeedUpper:
		case kWeather_TransDelta:
		case kWeather_SunGlare:
		case kWeather_SunDamage: {
			ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &floatVal, &weather);
			intVal = (floatVal * 255.0);
			break;
		}

		// extract float
		case kWeather_FogDayNear:
		case kWeather_FogDayFar:
		case kWeather_FogNightNear:
		case kWeather_FogNightFar: {
			ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &floatVal, &weather);
			break;
		}

	   // extract int
		case kWeather_LightningFrequency: 
		case kWeather_PrecipType: {
			ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &intVal, &weather);
			break;
		}
		default:
			break;
	}

	if (!weather) return true;

	switch(which) {
		case kWeather_WindSpeed:
			{
				weather->windSpeed = intVal;
				break;
			}
		case kWeather_CloudSpeedLower:
			{
				weather->cloudSpeedLower = intVal;
				break;
			}
		case kWeather_CloudSpeedUpper:
			{
				weather->cloudSpeedUpper = intVal;
				break;
			}
		case kWeather_TransDelta:
			{
				weather->transDelta = intVal;
				break;
			}
		case kWeather_SunGlare:
			{
				weather->sunGlare = intVal;
				break;
			}

		case kWeather_SunDamage:
			{
				weather->sunDamage = intVal;
				break;
			}
		case kWeather_FogDayNear:
			{
				weather->fogDay.nearFog = floatVal;
				break;
			}
		case kWeather_FogDayFar:
			{
				weather->fogDay.farFog = floatVal;
				break;
			}
		case kWeather_FogNightNear:
			{
				weather->fogNight.nearFog = floatVal;
				break;
			}
		case kWeather_FogNightFar:
			{
				weather->fogNight.farFog = floatVal;
				break;
			}

		case kWeather_LightningFrequency:
			{
				if (intVal > 255) intVal = 255;
				if (intVal == 0) intVal = 1; // crashes if intVal == 0;
				weather->lightningFrequency = intVal;
				break;
			}
	}
	return true;

}

static bool Cmd_SetWeatherLightningFrequency_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_LightningFrequency);
}

static bool Cmd_SetWeatherWindSpeed_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_WindSpeed);
}

static bool Cmd_SetWeatherCloudSpeedLower_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_CloudSpeedLower);
}

static bool Cmd_SetWeatherCloudSpeedUpper_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_CloudSpeedUpper);
}

static bool Cmd_SetWeatherTransDelta_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_TransDelta);
}

static bool Cmd_SetWeatherSunGlare_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_SunGlare);
}

static bool Cmd_SetWeatherSunDamage_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_SunDamage);
}

static bool Cmd_SetWeatherFogDayNear_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogDayNear);
}

static bool Cmd_SetWeatherFogDayFar_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogDayFar);
}

static bool Cmd_SetWeatherFogNightNear_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogNightNear);
}

static bool Cmd_SetWeatherFogNightFar_Execute(COMMAND_ARGS)
{
	return SetWeatherValue_Execute(PASS_COMMAND_ARGS, kWeather_FogNightFar);
}

static bool Cmd_GetWeatherClassification_Execute(COMMAND_ARGS)
{
	TESWeather* weather = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &weather) && weather)
	{
		switch (weather->precipType)
		{
		case TESWeather::kType_Rainy:
			*result = 3;
			break;
		case TESWeather::kType_Snow:
			*result = 4;
			break;
		default:
			*result = weather->precipType;
		}
	}

	return true;
}

static bool Cmd_GetWeatherOverride_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	Sky* sky = Sky::GetSingleton();
	if (sky && sky->weatherOverride) {
		*refResult = sky->weatherOverride->refID;
	}

	return true;
}

#endif

CommandInfo kCommandInfo_GetCurrentWeatherID =
{
	"GetCurrentWeatherID",
	"",
	0,
	"returns the form ID of the current weather",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetCurrentWeatherID_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCurrentClimateID =
{
	"GetCurrentClimateID",
	"",
	0,
	"returns the form ID of the current climate",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetCurrentClimateID_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


static ParamInfo kParams_OneWeatherType[] =
{
	{	"weather",	kParamType_WeatherID,	0 },
};

CommandInfo kCommandInfo_GetWeatherInfo =
{
	"GetWeatherInfo",
	"",
	0,
	"returns weather info",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherInfo_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateSunriseBegin =
{
	"GetClimateSunriseBegin",
	"GetSunriseBegin",
	0,
	"returns the sunrise begin time for the specified climate",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateSunriseBegin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateSunriseEnd =
{
	"GetClimateSunriseEnd",
	"GetSunriseEnd",
	0,
	"returns the sunrise end time for the specified climate",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateSunriseEnd_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateSunsetBegin =
{
	"GetClimateSunsetBegin",
	"GetSunsetBegin",
	0,
	"returns the sunset begin time for the specified climate",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateSunsetBegin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateSunsetEnd =
{
	"GetClimateSunsetEnd",
	"GetSunsetEnd",
	0,
	"returns the sunset end time for the specified climate",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateSunsetEnd_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateMoonPhaseLength =
{
	"GetClimateMoonPhaseLength",
	"GetMoonPhaseLength",
	0,
	"returns the length of the moon phase",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateMoonPhaseLength_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateHasMasser =
{
	"GetClimateHasMasser",
	"HasMasser",
	0,
	"returns 1 if the specified climate shows Masser",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateHasMasser_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateHasSecunda =
{
	"GetClimateHasSecunda",
	"HasSecunda",
	0,
	"returns 1 if the specified climate shows Secunda",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateHasSecunda_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetClimateInteger[2] =
{
	{	"value",	kParamType_Integer,		0 },
	{	"weather",	kParamType_WeatherID,	0 },
};

CommandInfo kCommandInfo_SetClimateSunriseBegin =
{
	"SetClimateSunriseBegin",
	"SetSunriseBegin",
	0,
	"sets the sunrise begin time for the specified climate",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateSunriseBegin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateSunriseEnd =
{
	"SetClimateSunriseEnd",
	"SetSunriseEnd",
	0,
	"sets the sunrise end time for the specified climate",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateSunriseEnd_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateSunsetBegin =
{
	"SetClimateSunsetBegin",
	"SetSunsetBegin",
	0,
	"sets the sunset begin time for the specified climate",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateSunsetBegin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateSunsetEnd =
{
	"SetClimateSunsetEnd",
	"SetSunsetEnd",
	0,
	"sets the sunset end time for the specified climate",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateSunsetEnd_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateMoonPhaseLength =
{
	"SetClimateMoonPhaseLength",
	"SetMoonPhaseLength",
	0,
	"sets the length of the moon phase",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateMoonPhaseLength_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateHasMasser =
{
	"SetClimateHasMasser",
	"",
	0,
	"sets whether the specified climate shows Masser",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateHasMasser_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateHasSecunda =
{
	"SetClimateHasSecunda",
	"",
	0,
	"marks the climate has showing secunda or not",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateHasSecunda_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClimateVolatility =
{
	"GetClimateVolatility",
	"",
	0,
	"returns the climate's volatility",
	0,
	2,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetClimateVolatility_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClimateVolatility =
{
	"SetClimateVolatility",
	"",
	0,
	"changes the climate's volatility",
	0,
	2,
	kParams_SetClimateInteger,
	HANDLER(Cmd_SetClimateVolatility_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherWindSpeed =
{
	"GetWeatherWindSpeed",
	"GetWindSpeed",
	0,
	"returns the wind speed of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherWindSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherCloudSpeedLower =
{
	"GetWeatherCloudSpeedLower",
	"GetCloudSpeedLower",
	0,
	"returns the speed of the lower cloud levels",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherCloudSpeedLower_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherCloudSpeedUpper =
{
	"GetWeatherCloudSpeedUpper",
	"GetCloudSpeedUpper",
	0,
	"returns the wind speed of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherCloudSpeedUpper_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherTransDelta =
{
	"GetWeatherTransDelta",
	"GetTransDelta",
	0,
	"returns the trans delts of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherTransDelta_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherSunGlare =
{
	"GetWeatherSunGlare",
	"GetSunGlare",
	0,
	"returns the sun glare of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherSunGlare_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherSunDamage =
{
	"GetWeatherSunDamage",
	"GetSunDamage",
	0,
	"returns the sun damage of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherSunDamage_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherFogDayNear =
{
	"GetWeatherFogDayNear",
	"GetFogDayNear",
	0,
	"returns the near fog in the day of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherFogDayNear_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherFogDayFar =
{
	"GetWeatherFogDayFar",
	"GetFogDayFar",
	0,
	"returns the far fog in the day of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherFogDayFar_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherFogNightNear =
{
	"GetWeatherFogNightNear",
	"GetFogNightNear",
	0,
	"returns the near fog in the night of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherFogNightNear_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherFogNightFar =
{
	"GetWeatherFogNightFar",
	"GetFogNightFar",
	0,
	"returns the far fog in the night of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherFogNightFar_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_WeatherInt[] =
{
	{	"value",	kParamType_Integer,		0 },
	{	"weather",	kParamType_WeatherID,	0 },
};


CommandInfo kCommandInfo_GetWeatherHDRValue =
{
	"GetWeatherHDRValue",
	"GetHDRValue",
	0,
	"returns the HDR Eye Adapt value of the specified weather",
	0,
	2,
	kParams_WeatherInt,
	HANDLER(Cmd_GetWeatherHDRValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetHDRValue[] =
{
	{	"nuVal",	kParamType_Float,		0 },
	{	"which",	kParamType_Integer,		0 },
	{	"weather",	kParamType_WeatherID,	0 },
};

CommandInfo kCommandInfo_SetWeatherHDRValue =
{
	"SetWeatherHDRValue",
	"SetHDRValue",
	0,
	"returns the HDR Eye Adapt value of the specified weather",
	0,
	3,
	kParams_SetHDRValue,
	HANDLER(Cmd_SetWeatherHDRValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


CommandInfo kCommandInfo_GetWeatherPrecipitationType =
{
	"GetWeatherPrecipitationType",
	"GetPrecipType",
	0,
	"returns the precipitation type of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherPrecipitationType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetWeatherLightningFrequency =
{
	"GetWeatherLightningFrequency",
	"GetLightningFrequency",
	0,
	"returns the lightning frequency of the specified weather",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_GetWeatherLightningFrequency_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetWeatherColor[] =
{
	{	"rgb",		kParamType_Integer,		0 },
	{	"whichColor", kParamType_Integer,	0 },
	{	"weather",	kParamType_WeatherID,	0 },
	{	"time",		kParamType_Integer,		1 },
};

CommandInfo kCommandInfo_GetWeatherColor =
{
	"GetWeatherColor",
	"",
	0,
	"gets the specified color of the specified weather",
	0,
	4,
	kParams_GetWeatherColor,
	HANDLER(Cmd_GetWeatherColor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetWeatherColor[] =
{
	{	"red",		kParamType_Integer,		0 },
	{	"green",	kParamType_Integer,		0 },
	{	"blue",		kParamType_Integer,		0 },
	{	"whichColor", kParamType_Integer,	0 },
	{	"weather",	kParamType_WeatherID,	0 },
	{	"time",		kParamType_Integer,		1},
};

CommandInfo kCommandInfo_SetWeatherColor =
{
	"SetWeatherColor",
	"",
	0,
	"sets the specified color of the specified weather",
	0,
	6,
	kParams_SetWeatherColor,
	HANDLER(Cmd_SetWeatherColor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetWeatherFloat[] =
{
	{	"value",	kParamType_Float,		0 },
	{	"weather",	kParamType_WeatherID,	0 },
};

static ParamInfo kParams_SetWeatherInt[] =
{
	{	"value",	kParamType_Integer,		0 },
	{	"weather",	kParamType_WeatherID,	0 },
};


CommandInfo kCommandInfo_SetWeatherLightningFrequency =
{
	"SetWeatherLightningFrequency",
	"SetLightningFrequency",
	0,
	"sets the lightning color of the specified weather",
	0,
	2,
	kParams_SetWeatherInt,
	HANDLER(Cmd_SetWeatherLightningFrequency_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


CommandInfo kCommandInfo_SetWeatherWindSpeed =
{
	"SetWeatherWindSpeed",
	"SetWindSpeed",
	0,
	"sets the wind speed of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherWindSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherCloudSpeedLower =
{
	"SetWeatherCloudSpeedLower",
	"SetCloudSpeedLower",
	0,
	"sets the lower cloud speed of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherCloudSpeedLower_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherCloudSpeedUpper =
{
	"SetWeatherCloudSpeedUpper",
	"SetCloudSpeedUpper",
	0,
	"sets the upper cloud speed of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherCloudSpeedUpper_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherTransDelta =
{
	"SetWeatherTransDelta",
	"SetTransDelta",
	0,
	"sets the transition delta of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherTransDelta_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherSunGlare =
{
	"SetWeatherSunGlare",
	"SetSunGlare",
	0,
	"sets the sun glare of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherSunGlare_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherSunDamage =
{
	"SetWeatherSunDamage",
	"SetSunDamage",
	0,
	"sets the sun damage of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherSunDamage_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherFogDayNear =
{
	"SetWeatherFogDayNear",
	"SetFogDayNear",
	0,
	"sets the fog day near of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherFogDayNear_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherFogDayFar =
{
	"SetWeatherFogDayFar",
	"SetFogDayFar",
	0,
	"sets the fog day far of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherFogDayFar_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherFogNightNear =
{
	"SetWeatherFogNightNear",
	"SetFogNightNear",
	0,
	"sets the fog night near of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherFogNightNear_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeatherFogNightFar =
{
	"SetWeatherFogNightFar",
	"SetFogNightFar",
	0,
	"sets the fog night far of the specified weather",
	0,
	2,
	kParams_SetWeatherFloat,
	HANDLER(Cmd_SetWeatherFogNightFar_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCurrentClimate =
{
	"SetCurrentClimate",
	"SetClimate",
	0,
	"sets the current climate to the passed value",
	0,
	1,
	kParams_OneWeatherType,
	HANDLER(Cmd_SetCurrentClimate_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RefreshCurrentClimate =
{
	"RefreshCurrentClimate",
	"RefreshClimate",
	0,
	"refreshes the current climate",
	0,
	0,
	0,
	HANDLER(Cmd_RefreshCurrentClimate_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetWorldspaceClimate[] =
{
	{	"worldspace",	kParamType_WorldSpace,	0 },
	{	"climate",		kParamType_ObjectRef,	0 },
};

CommandInfo kCommandInfo_SetWorldspaceClimate =
{
	"SetWorldspaceClimate",
	"",
	0,
	"sets the climate of a worldspace",
	0,
	2,
	kParams_SetWorldspaceClimate,
	HANDLER(Cmd_SetWorldspaceClimate_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetWeatherClassification, returns the precip type, 0, 1, kParams_OneWeatherType);
DEFINE_COMMAND(GetWeatherOverride, returns the currently overriding weather, 0, 0, NULL);
