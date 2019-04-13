#include "..\Common\inc\script.h"
#include "..\Common\inc\keyboard.h"
#include <cstdio>
#include <algorithm>
#include <string>
#include <ctime>

constexpr unsigned MAX_NAME = 64;

//#define DEBUG

#if defined(DEBUG)
char text[99];

__forceinline void update_status_text()
{
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.3f, 0.3f);
    UI::SET_TEXT_COLOUR(200, 200, 200, 255);
    UI::SET_TEXT_WRAP(0.0f, 1.0f);
    UI::SET_TEXT_DROPSHADOW(3, 0, 0, 0, 0);
    UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
    UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
    UI::END_TEXT_COMMAND_DISPLAY_TEXT(0.85f, 0.05f);
}
#endif

enum WEATHERS
{
    EXTRASUNNY,
    CLEAR,
    CLOUDS,
    SMOG,
    NEUTRAL,
    SNOWLIGHT,
    FOGGY,
    OVERCAST,
    RAIN,
    THUNDER,
    CLEARING,
    SNOW,
    BLIZZARD,
    XMAS,
    HALLOWEEN,
    W_MAX
};

enum CLOUDTYPES
{
    ALTOSTRATUS,
    CIRROCUMULUS,
    CIRRUS,
    NIMBUS1,
    NIMBUS2,
    CONTRAILS,
    HORIZON1,
    HORIZON2,
    NIMBUS3,
    HORIZON3,
    NIMBUS4,
    PUFFS,
    RAINY,
    SHOWER,
    NO_CLOUDS,
    STORMY,
    STRATOCUMULUS,
    STRIPEY,
    WISPY,
    C_MAX
};

struct Weather
{
    std::string name;
    unsigned    cloudType;
    bool        modifierEnabled;
    float       modifierStrength;
    float       chance;
    float       sumOfChance;
};

const std::string cloudTypes[C_MAX]   = { "altostratus", "cirrocumulus", "Cirrus", "Clear 01",
                                          "Cloudy 01", "Contrails", "Horizon", "horizonband1",
                                          "horizonband2", "horizonband3", "Nimbus", "Puffs",
                                          "RAIN", "shower", "Snowy 01", "Stormy 01",
                                          "stratoscumulus", "Stripey", "Wispy" };

const std::string weatherTypes[W_MAX] = { "EXTRASUNNY", "CLEAR", "CLOUDS", "SMOG",
                                          "NEUTRAL", "SNOWLIGHT", "FOGGY", "OVERCAST",
                                          "RAIN", "THUNDER", "CLEARING", "SNOW",
                                          "BLIZZARD", "XMAS", "HALLOWEEN" };

const std::string cloudCodes[C_MAX]   = { "ALTOSTRATUS", "CIRROCUMULUS", "CIRRUS", "NIMBUS1",
                                          "NIMBUS2", "CONTRAILS", "HORIZON1", "HORIZON2",
                                          "NIMBUS3", "HORIZON3", "NIMBUS4", "PUFFS",
                                          "RAINY", "SHOWER", "NO_CLOUDS", "STORMY",
                                          "STRATOCUMULUS", "STRIPEY", "WISPY" };

bool         modEnabled                          = true;
bool         realTimeDuration                    = true;
bool         changingWeatherAfterSleepingEnabled = true;
bool         freezeTime                          = false;
unsigned int toggleKey                           = 0x33;
unsigned int reloadKey                           = 0x34;


Hash    weatherHashes[W_MAX] = {};
Weather weather[W_MAX]       = {};

unsigned GetPrivateProfileHex(const char* sectionName, const char* keyName, const char* defaultValue, const char* fileName)
{
    char sValue[MAX_NAME];
    GetPrivateProfileString(sectionName, keyName, defaultValue, sValue, MAX_NAME, fileName);
    return strtol(sValue, nullptr, 16);
}

float GetPrivateProfileFloat(const char* sectionName, const char* keyName, const char* defaultValue, const char* fileName)
{
    char sValue[MAX_NAME];
    GetPrivateProfileString(sectionName, keyName, defaultValue, sValue, MAX_NAME, fileName);
    return strtof(sValue, nullptr);
}

bool GetPrivateProfileBool(const char* sectionName, const char* keyName, const bool defaultValue, const char* fileName)
{
    return (GetPrivateProfileInt(sectionName, keyName, defaultValue ? 1 : 0, fileName) == 1 ? true : false);
}

void loadConfigFromFile(const char* filePath)
{
    char sValue[MAX_NAME];

    toggleKey = GetPrivateProfileHex("KEYS", "ToggleKey", "0x33", filePath);
    reloadKey = GetPrivateProfileHex("KEYS", "ReloadKey", "0x34", filePath);

    modEnabled                          = GetPrivateProfileBool("COMMON", "EnableMod", true, filePath);
    realTimeDuration                    = GetPrivateProfileBool("COMMON", "EnableRealTimeDuration", true, filePath);
    changingWeatherAfterSleepingEnabled = GetPrivateProfileBool("COMMON", "EnableChangingWeatherAfterSleeping", true, filePath);
    freezeTime                          = GetPrivateProfileBool("COMMON", "FreezeTime", false, filePath);

    GetPrivateProfileString("EXTRASUNNY", "ModifierName", "glasses_yellow", sValue, MAX_NAME, filePath);
    weather[EXTRASUNNY].name = sValue;
    weather[EXTRASUNNY].modifierEnabled = GetPrivateProfileBool("EXTRASUNNY", "Enable", true, filePath);
    weather[EXTRASUNNY].modifierStrength = GetPrivateProfileFloat("EXTRASUNNY", "ModifierStrength", "1.0", filePath);
    weather[EXTRASUNNY].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "ExtrasunnyChance", "10.0", filePath);
    GetPrivateProfileString("EXTRASUNNY", "Clouds", "NO_CLOUDS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[EXTRASUNNY].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("CLEAR", "ModifierName", "glasses_Darkblue", sValue, MAX_NAME, filePath);
    weather[CLEAR].name = sValue;
    weather[CLEAR].modifierEnabled = GetPrivateProfileBool("CLEAR", "Enable", true, filePath);
    weather[CLEAR].modifierStrength = GetPrivateProfileFloat("CLEAR", "ModifierStrength", "1.0", filePath);
    weather[CLEAR].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "ClearChance", "15.0", filePath);
    GetPrivateProfileString("CLEAR", "Clouds", "NIMBUS1", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[CLEAR].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("NEUTRAL", "ModifierName", "glasses_VISOR", sValue, MAX_NAME, filePath);
    weather[NEUTRAL].name = sValue;
    weather[NEUTRAL].modifierEnabled = GetPrivateProfileBool("NEUTRAL", "Enable", true, filePath);
    weather[NEUTRAL].modifierStrength = GetPrivateProfileFloat("NEUTRAL", "ModifierStrength", "1.0", filePath);
    weather[NEUTRAL].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "NeutralChance", "0.0", filePath);
    GetPrivateProfileString("NEUTRAL", "Clouds", "NO_CLOUDS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[NEUTRAL].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("SMOG", "ModifierName", "glasses_black", sValue, MAX_NAME, filePath);
    weather[SMOG].name = sValue;
    weather[SMOG].modifierEnabled = GetPrivateProfileBool("SMOG", "Enable", true, filePath);
    weather[SMOG].modifierStrength = GetPrivateProfileFloat("SMOG", "ModifierStrength", "1.0", filePath);
    weather[SMOG].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "SmogChance", "15.0", filePath);
    GetPrivateProfileString("SMOG", "Clouds", "CIRROCUMULUS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[SMOG].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("FOGGY", "ModifierName", "glasses_brown", sValue, MAX_NAME, filePath);
    weather[FOGGY].name = sValue;
    weather[FOGGY].modifierEnabled = GetPrivateProfileBool("FOGGY", "Enable", true, filePath);
    weather[FOGGY].modifierStrength = GetPrivateProfileFloat("FOGGY", "ModifierStrength", "1.0", filePath);
    weather[FOGGY].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "FoggyChance", "4.0", filePath);
    GetPrivateProfileString("FOGGY", "Clouds", "ALTOSTRATUS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[FOGGY].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("OVERCAST", "ModifierName", "glasses_blue", sValue, MAX_NAME, filePath);
    weather[OVERCAST].name = sValue;
    weather[OVERCAST].modifierEnabled = GetPrivateProfileBool("OVERCAST", "Enable", true, filePath);
    weather[OVERCAST].modifierStrength = GetPrivateProfileFloat("OVERCAST", "ModifierStrength", "1.0", filePath);
    weather[OVERCAST].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "OvercastChance", "9.0", filePath);
    GetPrivateProfileString("OVERCAST", "Clouds", "ALTOSTRATUS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[OVERCAST].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("CLOUDS", "ModifierName", "glasses_red", sValue, MAX_NAME, filePath);
    weather[CLOUDS].name = sValue;
    weather[CLOUDS].modifierEnabled = GetPrivateProfileBool("CLOUDS", "Enable", true, filePath);
    weather[CLOUDS].modifierStrength = GetPrivateProfileFloat("CLOUDS", "ModifierStrength", "1.0", filePath);
    weather[CLOUDS].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "CloudsChance", "15.0", filePath);
    GetPrivateProfileString("CLOUDS", "Clouds", "PUFFS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[CLOUDS].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("CLEARING", "ModifierName", "glasses_green", sValue, MAX_NAME, filePath);
    weather[CLEARING].name = sValue;
    weather[CLEARING].modifierEnabled = GetPrivateProfileBool("CLEARING", "Enable", true, filePath);
    weather[CLEARING].modifierStrength = GetPrivateProfileFloat("CLEARING", "ModifierStrength", "1.0", filePath);
    weather[CLEARING].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "ClearingChance", "5.0", filePath);
    GetPrivateProfileString("CLEARING", "Clouds", "SHOWER", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[CLEARING].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("RAIN", "ModifierName", "glasses_yellow", sValue, MAX_NAME, filePath);
    weather[RAIN].name = sValue;
    weather[RAIN].modifierEnabled = GetPrivateProfileBool("RAIN", "Enable", true, filePath);
    weather[RAIN].modifierStrength = GetPrivateProfileFloat("RAIN", "ModifierStrength", "1.0", filePath);
    weather[RAIN].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "RainChance", "18.0", filePath);
    GetPrivateProfileString("RAIN", "Clouds", "RAINY", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[RAIN].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("THUNDER", "ModifierName", "glasses_purple", sValue, MAX_NAME, filePath);
    weather[THUNDER].name = sValue;
    weather[THUNDER].modifierEnabled = GetPrivateProfileBool("THUNDER", "Enable", true, filePath);
    weather[THUNDER].modifierStrength = GetPrivateProfileFloat("THUNDER", "ModifierStrength", "1.0", filePath);
    weather[THUNDER].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "ThunderChance", "9.0", filePath);
    GetPrivateProfileString("THUNDER", "Clouds", "STORMY", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[THUNDER].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("SNOW", "ModifierName", "glasses_indigo", sValue, MAX_NAME, filePath);
    weather[SNOW].name = sValue;
    weather[SNOW].modifierEnabled = GetPrivateProfileBool("SNOW", "Enable", true, filePath);
    weather[SNOW].modifierStrength = GetPrivateProfileFloat("SNOW", "ModifierStrength", "1.0", filePath);
    weather[SNOW].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "SnowChance", "0.0", filePath);
    GetPrivateProfileString("SNOW", "Clouds", "SHOWER", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[SNOW].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("BLIZZARD", "ModifierName", "glasses_pink", sValue, MAX_NAME, filePath);
    weather[BLIZZARD].name = sValue;
    weather[BLIZZARD].modifierEnabled = GetPrivateProfileBool("BLIZZARD", "Enable", true, filePath);
    weather[BLIZZARD].modifierStrength = GetPrivateProfileFloat("BLIZZARD", "ModifierStrength", "1.0", filePath);
    weather[BLIZZARD].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "BlizzardChance", "0.0", filePath);
    GetPrivateProfileString("BLIZZARD", "Clouds", "STORMY", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[BLIZZARD].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("SNOWLIGHT", "ModifierName", "glasses_orange", sValue, MAX_NAME, filePath);
    weather[SNOWLIGHT].name = sValue;
    weather[SNOWLIGHT].modifierEnabled = GetPrivateProfileBool("SNOWLIGHT", "Enable", true, filePath);
    weather[SNOWLIGHT].modifierStrength = GetPrivateProfileFloat("SNOWLIGHT", "ModifierStrength", "1.0", filePath);
    weather[SNOWLIGHT].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "SnowlightChance", "0.0", filePath);
    GetPrivateProfileString("SNOWLIGHT", "Clouds", "WISPY", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[SNOWLIGHT].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("XMAS", "ModifierName", "glasses_Scuba", sValue, MAX_NAME, filePath);
    weather[XMAS].name = sValue;
    weather[XMAS].modifierEnabled = GetPrivateProfileBool("XMAS", "Enable", true, filePath);
    weather[XMAS].modifierStrength = GetPrivateProfileFloat("XMAS", "ModifierStrength", "1.0", filePath);
    weather[XMAS].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "XmasChance", "0.0", filePath);
    GetPrivateProfileString("XMAS", "Clouds", "ALTOSTRATUS", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[XMAS].cloudType = i;
            break;
        }
    }

    GetPrivateProfileString("HALLOWEEN", "ModifierName", "sunglasses", sValue, MAX_NAME, filePath);
    weather[HALLOWEEN].name = sValue;
    weather[HALLOWEEN].modifierEnabled = GetPrivateProfileBool("HALLOWEEN", "Enable", true, filePath);
    weather[HALLOWEEN].modifierStrength = GetPrivateProfileFloat("HALLOWEEN", "ModifierStrength", "1.0", filePath);
    weather[HALLOWEEN].chance = GetPrivateProfileFloat("WEATHER_AFTER_SLEEPING", "HalloweenChance", "0.0", filePath);
    GetPrivateProfileString("HALLOWEEN", "Clouds", "STORMY", sValue, MAX_NAME, filePath);
    for (unsigned i = 0; i < C_MAX; ++i)
    {
        if (cloudCodes[i] == sValue)
        {
            weather[HALLOWEEN].cloudType = i;
            break;
        }
    }

    for (int i = 0; i < W_MAX; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            weather[i].sumOfChance += weather[j].chance;
        }
    }
}

unsigned SetWeatherAndClouds(const Hash weatherNow, const bool changeClouds, const float time)
{
    for (unsigned i = 0; i < W_MAX; ++i)
    {
        if (weatherHashes[i] == weatherNow && weather[i].modifierEnabled)
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(const_cast<char*>(weather[i].name.c_str()));
            GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(weather[i].modifierStrength);

            // Changing clouds.
            if (changeClouds)
            {
                for (int j = 0; j < C_MAX; ++j)
                {
                    GAMEPLAY::UNLOAD_CLOUD_HAT(const_cast<char*>(cloudTypes[j].c_str()), time);
                }
            }
            return i;
        }
    }
    return 0;
}

void ScriptMain()
{
    char filePath[MAX_PATH] = {};

    GetModuleFileName(nullptr, filePath, MAX_PATH);
    for (size_t i = strlen(filePath); i > 0; --i)
    {
        if (filePath[i] == '\\')
        {
            filePath[i] = '\0';
            break;
        }
    }

    strcat_s(filePath, "\\timecyclemods.ini");

    weatherHashes[EXTRASUNNY] = GAMEPLAY::GET_HASH_KEY("extrasunny");
    weatherHashes[CLEAR]      = GAMEPLAY::GET_HASH_KEY("clear");
    weatherHashes[CLOUDS]     = GAMEPLAY::GET_HASH_KEY("clouds");
    weatherHashes[SMOG]       = GAMEPLAY::GET_HASH_KEY("smog");
    weatherHashes[NEUTRAL]    = GAMEPLAY::GET_HASH_KEY("neutral");
    weatherHashes[SNOWLIGHT]  = GAMEPLAY::GET_HASH_KEY("snowlight");
    weatherHashes[FOGGY]      = GAMEPLAY::GET_HASH_KEY("foggy");
    weatherHashes[OVERCAST]   = GAMEPLAY::GET_HASH_KEY("overcast");
    weatherHashes[RAIN]       = GAMEPLAY::GET_HASH_KEY("rain");
    weatherHashes[THUNDER]    = GAMEPLAY::GET_HASH_KEY("thunder");
    weatherHashes[CLEARING]   = GAMEPLAY::GET_HASH_KEY("clearing");
    weatherHashes[SNOW]       = GAMEPLAY::GET_HASH_KEY("snow");
    weatherHashes[BLIZZARD]   = GAMEPLAY::GET_HASH_KEY("blizzard");
    weatherHashes[XMAS]       = GAMEPLAY::GET_HASH_KEY("xmas");
    weatherHashes[HALLOWEEN]  = GAMEPLAY::GET_HASH_KEY("halloween");

    Hash        currentWeather             = weatherHashes[EXTRASUNNY];
    Hash        nextWeather                = weatherHashes[EXTRASUNNY];
    float       transition                 = 0.0f;
    DWORD       lastTick                   = GetTickCount() + 1000;
    bool        isTimePaused               = false;
    std::time_t diffTimestamp              = 0;
    int         currentHour                = TIME::GET_CLOCK_HOURS();
    bool        playerIsControlled         = false;

    GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION(&currentWeather, &nextWeather, &transition);

    loadConfigFromFile(filePath);

    while (true)
    {
        const Hash   previousCurrentWeather     = currentWeather;
        const Hash   previousNextWeather        = nextWeather;
        const float  previousTransition         = transition;
        const bool   previousPlayerIsControlled = playerIsControlled;
        const Player playerId                   = PLAYER::GET_PLAYER_INDEX();

        playerIsControlled = PLAYER::IS_PLAYER_CONTROL_ON(playerId) ? true : false;

        GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION(&currentWeather, &nextWeather, &transition);

        const Hash weatherNow = (transition < 0.45f) ? currentWeather : nextWeather;

        // Reload configuration.
        if (IsKeyJustUp(reloadKey))
        {
            loadConfigFromFile(filePath);
        }

        // Enable / disable modification.
        if (IsKeyJustUp(toggleKey))
        {
            modEnabled = !modEnabled;
            if (!modEnabled)
            {
                GRAPHICS::CLEAR_TIMECYCLE_MODIFIER();
            }
        }

        // Real time duration is enabled.
        if (realTimeDuration)
        {
            if (!isTimePaused)
            {
                isTimePaused = true;
                TIME::PAUSE_CLOCK(isTimePaused);

                std::time_t timestamp   = 1000000;
                std::tm     gmTime      = *std::gmtime(&timestamp);
                std::time_t gmTimestamp = std::mktime(&gmTime);

                diffTimestamp = timestamp - gmTimestamp;

                #if defined(DEBUG)
                //TIME::SET_CLOCK_DATE(9, 5, 2018);
                //TIME::SET_CLOCK_TIME(21, 37, 1);
                #endif
            }
            if (lastTick < GetTickCount())
            {
                lastTick = GetTickCount() + 1000;

                // Get current time.
                int year   = TIME::GET_CLOCK_YEAR();
                int month  = TIME::GET_CLOCK_MONTH();
                int day    = TIME::GET_CLOCK_DAY_OF_MONTH();
                int hour   = TIME::GET_CLOCK_HOURS();
                int minute = TIME::GET_CLOCK_MINUTES();
                int second = TIME::GET_CLOCK_SECONDS();

                std::tm time = {};

                time.tm_year  = year - 1900;
                time.tm_mon   = month - 1;
                time.tm_mday  = day;
                time.tm_hour  = hour;
                time.tm_min   = minute;
                time.tm_sec   = second;
                time.tm_isdst = 0;

                // Convert current time to unix timestamp.
                std::time_t timestamp = std::mktime(&time);

                // Add one second.
                ++timestamp;

                // Add a difference between time zones.
                timestamp += diffTimestamp;

                // Convert unix timestamp to time.
                time = *std::gmtime(&timestamp);

                // Set current time.
                year   = time.tm_year + 1900;
                month  = time.tm_mon + 1;
                day    = time.tm_mday;
                hour   = time.tm_hour;
                minute = time.tm_min;
                second = time.tm_sec;

                TIME::SET_CLOCK_DATE(day, month, year);
                TIME::SET_CLOCK_TIME(hour, minute, second);

                #if defined(DEBUG)
                sprintf_s(text, "%d %d %d %d:%d:%d", year, month, day, hour, minute, second);
                #endif
            }
        }
        else
        {
            if (freezeTime)
            {
                TIME::PAUSE_CLOCK(true);
            }
            else if (isTimePaused)
            {
                isTimePaused = false;
                TIME::PAUSE_CLOCK(isTimePaused);
            }
        }

        if (changingWeatherAfterSleepingEnabled)
        {
            const int previousHour = currentHour;

            currentHour = TIME::GET_CLOCK_HOURS();

            if ((currentHour != 0 || previousHour != 23) && (currentHour != 23 || previousHour != 0))
            {
                #if defined(DEBUG)
                static int cos = 0;
                static int random = 0;
                //sprintf(text, "%d", control);
                #endif

                if (!playerIsControlled && ((currentHour - previousHour) >= 6 || previousHour - currentHour > 1))
                {
                    const int rand = GAMEPLAY::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(weather[HALLOWEEN].sumOfChance));

                    #if defined(DEBUG)
                    ++cos;
                    random = rand;
                    #endif

                    for (int i = W_MAX - 1; i >= 0; --i)
                    {
                        if (weather[i].chance > 0.0f && static_cast<int>(weather[i].sumOfChance) < rand)
                        {
                            float trans = 0.0f;

                            GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weatherHashes[i], nextWeather, trans);

                            const unsigned clouds = weather[i].cloudType;

                            GAMEPLAY::LOAD_CLOUD_HAT(const_cast<char*>(cloudTypes[clouds].c_str()), 1.0f);

                            break;
                        }
                    }
                }

                #if defined(DEBUG)
                sprintf_s(text, "%d %d %f %f", cos, random, weather[HALLOWEEN].chance, weather[HALLOWEEN].sumOfChance);
                #endif
            }
        }

        // Apply changes if modification is enabled.
        if (modEnabled)
        {
            // Weather is changing naturally.
            if (previousTransition < 0.45f && transition >= 0.45f)
            {
                SetWeatherAndClouds(nextWeather, changingWeatherAfterSleepingEnabled, 30.0f);
            }

            // All modifiers have been cleaned.
            else if (!previousPlayerIsControlled && playerIsControlled || GRAPHICS::GET_TIMECYCLE_MODIFIER_INDEX() == -1)
            {
                const unsigned weatherIndex = SetWeatherAndClouds(weatherNow, true, 0.0f);
                const unsigned cloudsIndex  = weather[weatherIndex].cloudType;

                GAMEPLAY::LOAD_CLOUD_HAT(const_cast<char*>(cloudTypes[cloudsIndex].c_str()), 1.0f);
            }

            // Weather has been changed by trainer.
            else if ((transition > 0.0f && transition < 0.45f && previousCurrentWeather != currentWeather) ||
                (transition >= 0.45f && previousNextWeather != nextWeather))
            {
                SetWeatherAndClouds(weatherNow, changingWeatherAfterSleepingEnabled, 0.0f);
            }
        }

        #if defined(DEBUG)
        update_status_text();
        #endif
        WAIT(0);
    }
}

void UnloadScript()
{
    GRAPHICS::CLEAR_TIMECYCLE_MODIFIER();
    TIME::PAUSE_CLOCK(false);
}
