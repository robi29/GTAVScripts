#define _CRT_SECURE_NO_WARNINGS

#include "script.h"
#include "keyboard.h"

#include <cstdio>
#include <Psapi.h>

enum class Decor : int
{
    Float = 1,
    Bool,
    Int,
    Unknown,
    Time
};

enum class Weather : int
{
    Extrasunny = 0,
    Clear,
    Clouds,
    Smog,
    Neutral,
    Snowlight,
    Foggy,
    Overcast,
    Rain,
    Thunder,
    Clearing,
    Snow,
    Blizzard,
    Xmas,
    Halloween,
    Count
};

enum class Cloud
{
    Altostratus,
    Cirrocumulus,
    Cirrus,
    Nimbus1,
    Nimbus2,
    Contrails,
    Horizon1,
    Horizon2,
    Nimbus3,
    Horizon3,
    Nimbus4,
    Puffs,
    Rainy,
    Shower,
    NoClouds,
    Stormy,
    Stratocumulus,
    Stripey,
    Wispy,
    Count
};

constexpr unsigned int MAX_NAME = 64;

Any weather  = 0;
Any weather0 = 0;
Any weather1 = 0;

char path[MAX_PATH]     = "";
char nameFile[MAX_NAME] = "atmosphere.ini";
bool reload             = false;
bool onOff              = false;
bool debug              = false;

float progress              = 0.0;
bool isNormalZone           = true;
bool isCloudZoneStart       = false;
bool isCloudZoneEnd         = false;
bool isAboveCloudsZoneStart = false;
bool isAboveCloudsZoneEnd   = false;
bool isSpaceZoneStart       = false;
bool isSpace                = false;

Hash weatherHashes[static_cast<int>(Weather::Count)] = {};

const char cloudTypes[static_cast<int>(Cloud::Count)][15] = { "altostratus",    "cirrocumulus", "Cirrus",   "Clear 01",
                                                              "Cloudy 01",      "Contrails",    "Horizon",  "horizonband1",
                                                              "horizonband2",   "horizonband3", "Nimbus",   "Puffs",
                                                              "RAIN",           "shower",       "Snowy 01", "Stormy 01",
                                                              "stratoscumulus", "Stripey",      "Wispy" };

const char cloudCodes[static_cast<int>(Cloud::Count)][14] = { "ALTOSTRATUS",   "CIRROCUMULUS", "CIRRUS",    "NIMBUS1",
                                                              "NIMBUS2",       "CONTRAILS",    "HORIZON1",  "HORIZON2",
                                                              "NIMBUS3",       "HORIZON3",     "NIMBUS4",   "PUFFS",
                                                              "RAINY",         "SHOWER",       "NO_CLOUDS", "STORMY",
                                                              "STRATOCUMULUS", "STRIPEY",      "WISPY" };

const char weatherTypes[static_cast<int>(Weather::Count)][11] = { "EXTRASUNNY", "CLEAR",     "CLOUDS",   "SMOG",
                                                                  "NEUTRAL",    "SNOWLIGHT", "FOGGY",    "OVERCAST",
                                                                  "RAIN",       "THUNDER",   "CLEARING", "SNOW",
                                                                  "BLIZZARD",   "XMAS",      "HALLOWEEN" };

char weatherClouds[static_cast<int>(Weather::Count)][MAX_NAME] = { "Snowy 01",  "Puffs",       "Cloudy 01",   "Nimbus",
                                                                   "Snowy 01",  "Contrails",   "altostratus", "Stripey",
                                                                   "RAIN",      "Stormy 01",   "shower",      "altostratus",
                                                                   "Stormy 01", "altostratus", "Snowy 01" };

char normalZoneTimecycleModifier[static_cast<int>(Weather::Count)][MAX_NAME] = { "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                 "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                 "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                 "nextgen", "nextgen", "nextgen" };

char cloudZoneTimecycleModifier[static_cast<int>(Weather::Count)][MAX_NAME] = { "nextgen",   "nextgen",   "nextgen",   "nextgen",
                                                                                "nextgen",   "nextgen",   "cloudzone", "cloudzone",
                                                                                "cloudzone", "cloudzone", "cloudzone", "cloudzone",
                                                                                "cloudzone", "cloudzone", "cloudzone" };

char aboveCloudsZoneTimecycleModifier[static_cast<int>(Weather::Count)][MAX_NAME] = { "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                      "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                      "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                      "nextgen", "nextgen", "nextgen" };

char spaceZoneTimecycleModifier[static_cast<int>(Weather::Count)][MAX_NAME] = { "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                "nextgen", "nextgen", "nextgen", "nextgen",
                                                                                "nextgen", "nextgen", "nextgen" };

float aboveCloudsZoneTimecycleTransitionTime[static_cast<int>(Weather::Count)] = { 30.0f, 30.0f, 30.0f, 30.0f, 30.0f,
                                                                                   30.0f, 30.0f, 30.0f, 30.0f, 30.0f,
                                                                                   30.0f, 30.0f, 30.0f, 30.0f, 30.0f };

float spaceZoneTimecycleTransitionTime[static_cast<int>(Weather::Count)] = { 30.0f, 30.0f, 30.0f, 30.0f, 30.0f,
                                                                             30.0f, 30.0f, 30.0f, 30.0f, 30.0f,
                                                                             30.0f, 30.0f, 30.0f, 30.0f, 30.0f };

bool enableLightningAboveClouds[static_cast<int>(Weather::Count)] = { false, false, false, false, false,
                                                                      false, false, false, false, true,
                                                                      false, false, false, false, false };

bool enableLightningInSpace[static_cast<int>(Weather::Count)] = { false, false, false, false, false,
                                                                  false, false, false, false, false,
                                                                  false, false, false, false, false };

char lightningFrequency[static_cast<int>(Weather::Count)] = { 0, 0, 0, 0, 0,
                                                              0, 0, 0, 0, 5,
                                                              0, 0, 0, 0, 0 };

float normalZoneEnd[static_cast<int>(Weather::Count)] = { 800.0f, 800.0f, 800.0f, 800.0f, 800.0f,
                                                          800.0f, 800.0f, 800.0f, 800.0f, 800.0f,
                                                          800.0f, 800.0f, 800.0f, 800.0f, 800.0f };

float cloudZoneStart[static_cast<int>(Weather::Count)] = { 900.0f, 900.0f, 900.0f, 900.0f, 900.0f,
                                                           900.0f, 900.0f, 900.0f, 900.0f, 900.0f,
                                                           900.0f, 900.0f, 900.0f, 900.0f, 900.0f };

float cloudZoneEnd[static_cast<int>(Weather::Count)] = { 1200.0f, 1200.0f, 1200.0f, 1200.0f, 1200.0f,
                                                         1200.0f, 1200.0f, 1200.0f, 1200.0f, 1200.0f,
                                                         1200.0f, 1200.0f, 1200.0f, 1200.0f, 1200.0f };

float aboveCloudsZoneStart[static_cast<int>(Weather::Count)] = { 1400.0f, 1400.0f, 1400.0f, 1400.0f, 1400.0f,
                                                                 1400.0f, 1400.0f, 1400.0f, 1400.0f, 1400.0f,
                                                                 1400.0f, 1400.0f, 1400.0f, 1400.0f, 1400.0f };

float aboveCloudsZoneEnd[static_cast<int>(Weather::Count)] = { 2000.0f, 2000.0f, 2000.0f, 2000.0f, 2000.0f,
                                                               2000.0f, 2000.0f, 2000.0f, 2000.0f, 2000.0f,
                                                               2000.0f, 2000.0f, 2000.0f, 2000.0f, 2000.0f };

float spaceZoneStart[static_cast<int>(Weather::Count)] = { 3000.0f, 3000.0f, 3000.0f, 3000.0f, 3000.0f,
                                                           3000.0f, 3000.0f, 3000.0f, 3000.0f, 3000.0f,
                                                           3000.0f, 3000.0f, 3000.0f, 3000.0f, 3000.0f };

float cloudWindMultiplier[static_cast<int>(Weather::Count)] = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f,
                                                                2.0f, 2.0f, 2.0f, 2.0f, 2.0f,
                                                                2.0f, 2.0f, 2.0f, 2.0f, 2.0f };

float aboveWindMultiplier[static_cast<int>(Weather::Count)] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                                                                0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                                                                0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

float spaceWindMultiplier[static_cast<int>(Weather::Count)] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                                                0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                                                0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
 
Weather aboveWeatherOverride[static_cast<int>(Weather::Count)] = { Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny,
                                                                   Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny,
                                                                   Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny, Weather::Extrasunny };

Weather spaceWeatherOverride[static_cast<int>(Weather::Count)] = { Weather::Halloween, Weather::Halloween, Weather::Halloween, Weather::Halloween, Weather::Halloween,
                                                                   Weather::Halloween, Weather::Halloween, Weather::Halloween, Weather::Halloween, Weather::Halloween,
                                                                   Weather::Halloween, Weather::Halloween, Weather::Halloween, Weather::Halloween, Weather::Halloween };

bool defaultWindSpeed = true;
bool defaultRainFX    = true;
bool isDefaultWind    = false;
bool reloadModifier   = false;

float windSpeed = 1.0f;
float textScale = 0.3f;

unsigned reloadKey = 0x2E;
unsigned onOffKey  = 0x2D;
unsigned debugKey  = 0x23;

unsigned textColorRed   = 255;
unsigned textColorGreen = 245;
unsigned textColorBlue  = 195;

unsigned counter = 0;
unsigned random  = 0;

Weather currentWeatherType = Weather::Extrasunny;

DWORD statusTextDrawTicksMax = 0;
const unsigned textLines     = 6;
char text[textLines][99]     = {};

float posX = 0.03f;
float posY = 0.65f;

const float offset[textLines] = { 0.0000f, 0.2686f, 0.5371f, 0.8057f, 1.0071f, 1.2086f };// , 1.5109f };

__forceinline Weather& operator++(Weather& weather) // prefix ++
{
    weather = static_cast<Weather>(static_cast<int>(weather) + 1);
    return weather;
}

#define GET_DECORATOR_BOOL(name, var)                                                     \
        if (DECORATOR::DECOR_IS_REGISTERED_AS_TYPE(#name, static_cast<int>(Decor::Bool))) \
        {                                                                                 \
            boolDecorator = DECORATOR::DECOR_GET_BOOL(entity, #name);                     \
            if (boolDecorator == TRUE)                                                    \
            {                                                                             \
                var = true;                                                               \
                DECORATOR::DECOR_SET_BOOL(entity, #name, FALSE);                          \
            }                                                                             \
        }

__forceinline float lerp(float a, float b, float t)
{
    return (1 - t) * a + t * b;
}

__forceinline unsigned GetPrivateProfileHex(char* sectionName, char* keyName, char* defaultValue, char* fileName)
{
    char sValue[MAX_NAME];
    GetPrivateProfileString(sectionName, keyName, defaultValue, sValue, MAX_NAME, fileName);
    return strtol(sValue, NULL, 16);
}

__forceinline float GetPrivateProfileFloat(char* sectionName, char* keyName, char* defaultValue, char* fileName)
{
    char sValue[MAX_NAME];
    GetPrivateProfileString(sectionName, keyName, defaultValue, sValue, MAX_NAME, fileName);
    return strtof(sValue, NULL);
}

__forceinline bool GetPrivateProfileBool(char* sectionName, char* keyName, bool defaultValue, char* fileName)
{
    return (GetPrivateProfileInt(sectionName, keyName, defaultValue ? 1 : 0, fileName) == 1 ? true : false);
}

__forceinline void update_status_text()
{
    for (unsigned i = 0; i < textLines; i++)
    {
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(textScale, textScale);
        UI::SET_TEXT_COLOUR(textColorRed, textColorGreen, textColorBlue, 255);
        UI::SET_TEXT_WRAP(0.0f, 1.0f);
        UI::SET_TEXT_DROPSHADOW(3, 0, 0, 0, 0);
        UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
        UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
        UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text[i]);
        UI::END_TEXT_COMMAND_DISPLAY_TEXT(posX, posY + offset[i] * textScale);
    }
}

__forceinline bool isSunnyWeather(Hash w)
{
    for (unsigned i = 0; i < 6; i++)
        if (weatherHashes[i] == w)
            return true;

    return false;
}

__forceinline Weather getWeatherType(Hash w)
{
    for (Weather i = Weather::Extrasunny; i < Weather::Count; ++i)
        if (weatherHashes[static_cast<int>(i)] == w)
            return i;

    return Weather::Extrasunny;
}

__forceinline void resetCloudHat()
{
    GAMEPLAY::_CLEAR_CLOUD_HAT();
    for (Weather i = Weather::Extrasunny; i < Weather::Count; ++i)
    {
        if (weather == weatherHashes[static_cast<int>(i)])
        {
            GAMEPLAY::LOAD_CLOUD_HAT(weatherClouds[static_cast<int>(i)], 0.0f);
            return;
        }
    }
    GAMEPLAY::LOAD_CLOUD_HAT(weatherClouds[static_cast<int>(Weather::Neutral)], 0.0f);
}

__forceinline void normalZoneUpdate()
{
    isNormalZone           = true;
    isCloudZoneStart       = false;
    isCloudZoneEnd         = false;
    isAboveCloudsZoneStart = false;
    isAboveCloudsZoneEnd   = false;
    isSpaceZoneStart       = false;
    isSpace                = false;
}

__forceinline void cloudZoneStartUpdate()
{
    isNormalZone           = false;
    isCloudZoneStart       = true;
    isCloudZoneEnd         = false;
    isAboveCloudsZoneStart = false;
    isAboveCloudsZoneEnd   = false;
    isSpaceZoneStart       = false;
    isSpace                = false;
}

__forceinline void cloudZoneEndUpdate()
{
    isNormalZone           = false;
    isCloudZoneStart       = false;
    isCloudZoneEnd         = true;
    isAboveCloudsZoneStart = false;
    isAboveCloudsZoneEnd   = false;
    isSpaceZoneStart       = false;
    isSpace                = false;
}

__forceinline void aboveCloudsZoneStartUpdate()
{
    isNormalZone           = false;
    isCloudZoneStart       = false;
    isCloudZoneEnd         = false;
    isAboveCloudsZoneStart = true;
    isAboveCloudsZoneEnd   = false;
    isSpaceZoneStart       = false;
    isSpace                = false;
}

__forceinline void aboveCloudsZoneEndUpdate()
{
    isNormalZone           = false;
    isCloudZoneStart       = false;
    isCloudZoneEnd         = false;
    isAboveCloudsZoneStart = false;
    isAboveCloudsZoneEnd   = true;
    isSpaceZoneStart       = false;
    isSpace                = false;
}

__forceinline void spaceZoneStartUpdate()
{
    isNormalZone           = false;
    isCloudZoneStart       = false;
    isCloudZoneEnd         = false;
    isAboveCloudsZoneStart = false;
    isAboveCloudsZoneEnd   = false;
    isSpaceZoneStart       = true;
    isSpace                = false;
}

__forceinline void spaceZoneUpdate()
{
    isNormalZone           = false;
    isCloudZoneStart       = false;
    isCloudZoneEnd         = false;
    isAboveCloudsZoneStart = false;
    isAboveCloudsZoneEnd   = false;
    isSpaceZoneStart       = false;
    isSpace                = true;
}

__forceinline void doLightningAboveClouds()
{
    if (counter > random)
    {
        if (enableLightningAboveClouds[static_cast<int>(currentWeatherType)])
        {
            GAMEPLAY::FORCE_LIGHTNING_FLASH();
        }
        counter = 0;
        random = rand() % (2000 / lightningFrequency[static_cast<int>(currentWeatherType)]);
    }
}

__forceinline void doLightningInSpace()
{
    if (counter > random)
    {
        if (enableLightningInSpace[static_cast<int>(currentWeatherType)])
        {
            GAMEPLAY::FORCE_LIGHTNING_FLASH();
        }
        counter = 0;
        random = rand() % (2000 / lightningFrequency[static_cast<int>(currentWeatherType)]);
    }
}

__forceinline void getDecorators(Entity entity)
{
    int   intDecorator;
    BOOL  boolDecorator;

    if (DECORATOR::DECOR_IS_REGISTERED_AS_TYPE("fileindex", static_cast<int>(Decor::Int)))
    {
        intDecorator = DECORATOR::DECOR_GET_INT(entity, "fileindex");

        if (intDecorator > 0)
        {
            char index[32];
            char name[256] = { "atmosphere" };
            _itoa(intDecorator, index, 10);
            strcat(name, index);
            strcat(name, ".ini");
            strcpy(nameFile, name);
        }
    }

    GET_DECORATOR_BOOL("reload", reload);
    GET_DECORATOR_BOOL("enabled", onOff);
    GET_DECORATOR_BOOL("debugmode", debug);
}

__forceinline bool isAllModifiersDisabled()
{
    return GRAPHICS::GET_TIMECYCLE_MODIFIER_INDEX() < 0;
}

__forceinline void update(Entity entity)
{
    register Vector3 coords;
    register float   progress;

    ++counter;

    if (PED::IS_PED_IN_ANY_VEHICLE(entity, 0))
        entity = PED::GET_VEHICLE_PED_IS_USING(entity);

    coords = ENTITY::GET_ENTITY_COORDS(entity, 0);

    if (coords.z < normalZoneEnd[static_cast<int>(currentWeatherType)]) // z < 800
    {
        GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION(&weather0, &weather1, &progress);
        currentWeatherType = getWeatherType((progress < 0.5f) ? weather0 : weather1);

        if (!isSunnyWeather(weather) && !isNormalZone && !defaultWindSpeed)
        {
            if (!isDefaultWind)
            {
                GAMEPLAY::SET_WIND_SPEED(-1.0f);
            }
            defaultWindSpeed = true;
        }
        if (!defaultRainFX)
        {
            GAMEPLAY::_SET_RAIN_FX_INTENSITY(-1.0);
            defaultRainFX = true;
        }
        if (isCloudZoneStart || isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(normalZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }

        if (coords.z > normalZoneEnd[static_cast<int>(currentWeatherType)] - 50.0f)
        {
            float transition = (normalZoneEnd[static_cast<int>(currentWeatherType)] - coords.z) / 50.0f;
            GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(transition);
        }
        else
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);
        }
        windSpeed = GAMEPLAY::GET_WIND_SPEED();

        normalZoneUpdate();

        sprintf_s(text[0], "normalzone\n");
    }
    else if (coords.z < cloudZoneStart[static_cast<int>(currentWeatherType)]) // z < 900
    {
        defaultWindSpeed = false;
        progress = (coords.z - normalZoneEnd[static_cast<int>(currentWeatherType)]) / (cloudZoneStart[static_cast<int>(currentWeatherType)] - normalZoneEnd[static_cast<int>(currentWeatherType)]);
        weather = (progress < 0.5f) ? weather0 : weather1;
        if (isNormalZone || isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(cloudZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(progress);
        if (!isSunnyWeather(weather))
        {
            if (!isDefaultWind)
            {
                GAMEPLAY::SET_WIND_SPEED(lerp(windSpeed, windSpeed*cloudWindMultiplier[static_cast<int>(currentWeatherType)], progress));
            }
        }
        else
        {
            windSpeed = GAMEPLAY::GET_WIND_SPEED();
        }
        if (isCloudZoneEnd)
        {
            resetCloudHat();
            GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
            GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weather0, weather1, progress);
            GRAPHICS::SET_TIMECYCLE_MODIFIER(cloudZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }

        doLightningAboveClouds();

        cloudZoneStartUpdate();

        sprintf_s(text[0], "cloudzonestart\n");
    }
    else if (coords.z < cloudZoneEnd[static_cast<int>(currentWeatherType)]) // z < 1200
    {
        progress = (coords.z - cloudZoneStart[static_cast<int>(currentWeatherType)]) / (cloudZoneEnd[static_cast<int>(currentWeatherType)] - cloudZoneStart[static_cast<int>(currentWeatherType)]);
        GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weather0, weather1, progress * (1.0f - progress));
        weather = (progress < 0.5f) ? weather0 : weather1;
        if (isCloudZoneStart || isAboveCloudsZoneStart || isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(cloudZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);
        if (!isSunnyWeather(weather))
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(progress);
            GAMEPLAY::LOAD_CLOUD_HAT("horsey", 0.0f);
            if (!isDefaultWind)
            {
                GAMEPLAY::SET_WIND_SPEED(cloudWindMultiplier[static_cast<int>(currentWeatherType)] * windSpeed);
            }
        }
        else
        {
            if (!defaultWindSpeed)
            {
                defaultWindSpeed = true;
                if (!isDefaultWind)
                {
                    GAMEPLAY::SET_WIND_SPEED(-1.0f);
                }
            }
            windSpeed = GAMEPLAY::GET_WIND_SPEED();
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f - progress);
        }

        doLightningAboveClouds();

        cloudZoneEndUpdate();

        sprintf_s(text[0], "cloudzoneend\n");
    }
    else if (coords.z < aboveCloudsZoneStart[static_cast<int>(currentWeatherType)]) // z < 1400
    {
        progress = (coords.z - cloudZoneEnd[static_cast<int>(currentWeatherType)]) / (aboveCloudsZoneStart[static_cast<int>(currentWeatherType)] - cloudZoneEnd[static_cast<int>(currentWeatherType)]);
        weather = (progress < 0.5f) ? weather0 : weather1;
        defaultWindSpeed = false;
        if (isCloudZoneEnd || isAboveCloudsZoneEnd || isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(cloudZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f - progress);
        if (!isSunnyWeather(weather))
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("horsey", 0.0f);
            if (!isDefaultWind)
            {
                GAMEPLAY::SET_WIND_SPEED(lerp(windSpeed*cloudWindMultiplier[static_cast<int>(currentWeatherType)], windSpeed*aboveWindMultiplier[static_cast<int>(currentWeatherType)], progress));
            }
        }
        else
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(progress);
            GAMEPLAY::LOAD_CLOUD_HAT("Snowy 01", 0.0f);
            if (!isDefaultWind)
            {
                GAMEPLAY::SET_WIND_SPEED(lerp(windSpeed, windSpeed*aboveWindMultiplier[static_cast<int>(currentWeatherType)], progress));
            }
        }
        const int weatherIndex = static_cast<int>(aboveWeatherOverride[static_cast<int>(currentWeatherType)]);
        GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weather, weatherHashes[weatherIndex], (progress * 3.0f > 1.0f) ? 1.0f : progress * 3.0f);

        doLightningAboveClouds();

        aboveCloudsZoneStartUpdate();

        sprintf_s(text[0], "abovecloudzonestart\n");
    }
    else if (coords.z < aboveCloudsZoneEnd[static_cast<int>(currentWeatherType)]) // z < 2000
    {
        const int weatherIndex = static_cast<int>(aboveWeatherOverride[static_cast<int>(currentWeatherType)]);
        GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weatherHashes[weatherIndex], weatherHashes[weatherIndex], 0.5f);
        if (!isSunnyWeather(weather))
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("horsey", 0.0f);
        }
        else
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("Snowy 01", 0.0f);
        }
        defaultRainFX = false;
        GAMEPLAY::_SET_RAIN_FX_INTENSITY(0.0f);
        if (!isDefaultWind)
        {
            GAMEPLAY::SET_WIND_SPEED(aboveWindMultiplier[static_cast<int>(currentWeatherType)] * windSpeed);
        }

        if (isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(aboveCloudsZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }
        if (reloadModifier)
        {
            GRAPHICS::SET_TRANSITION_TIMECYCLE_MODIFIER(aboveCloudsZoneTimecycleModifier[static_cast<int>(currentWeatherType)], aboveCloudsZoneTimecycleTransitionTime[static_cast<int>(currentWeatherType)]);
            reloadModifier = false;
        }
        if (isSpaceZoneStart || isAboveCloudsZoneStart)
        {
            if (!isSpaceZoneStart)
            {
                GRAPHICS::SET_TIMECYCLE_MODIFIER("nextgen");
            }
            reloadModifier = true;
        }
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);

        doLightningAboveClouds();

        aboveCloudsZoneEndUpdate();

        sprintf_s(text[0], "abovecloudzoneend\n");
    }
    else if (coords.z < spaceZoneStart[static_cast<int>(currentWeatherType)]) // z < 3000
    {
        progress = (coords.z - aboveCloudsZoneEnd[static_cast<int>(currentWeatherType)]) / (spaceZoneStart[static_cast<int>(currentWeatherType)] - aboveCloudsZoneEnd[static_cast<int>(currentWeatherType)]);
        if (isSpace)
        {
            GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
        }
        if (!isSunnyWeather(weather))
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("horsey", 0.0f);
        }
        else
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("Snowy 01", 0.0f);
        }
        defaultRainFX = false;
        GAMEPLAY::_SET_RAIN_FX_INTENSITY(0.0f);
        if (!isDefaultWind)
        {
            GAMEPLAY::SET_WIND_SPEED(lerp(windSpeed*aboveWindMultiplier[static_cast<int>(currentWeatherType)], 0.0f, progress));
        }
        const int aboveWeatherIndex = static_cast<int>(aboveWeatherOverride[static_cast<int>(currentWeatherType)]);
        const int spaceWeatherIndex = static_cast<int>(spaceWeatherOverride[static_cast<int>(currentWeatherType)]);
        GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weatherHashes[aboveWeatherIndex], weatherHashes[spaceWeatherIndex],
            SYSTEM::SIN((coords.z - aboveCloudsZoneEnd[static_cast<int>(currentWeatherType)]) / (spaceZoneStart[static_cast<int>(currentWeatherType)] - aboveCloudsZoneEnd[static_cast<int>(currentWeatherType)]) * 90));

        if (isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(spaceZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }
        if (isAboveCloudsZoneEnd)
        {
            GRAPHICS::SET_TRANSITION_TIMECYCLE_MODIFIER(spaceZoneTimecycleModifier[static_cast<int>(currentWeatherType)], spaceZoneTimecycleTransitionTime[static_cast<int>(currentWeatherType)]);
        }
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);

        doLightningInSpace();

        spaceZoneStartUpdate();

        sprintf_s(text[0], "spacezonestart\n");
    }
    else
    {
        if (!isSpace)
        {
            const int weatherIndex = static_cast<int>(spaceWeatherOverride[static_cast<int>(currentWeatherType)]);
            GAMEPLAY::SET_OVERRIDE_WEATHER(const_cast<char*>(weatherTypes[weatherIndex]));
        }
        if (!isSunnyWeather(weather))
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("horsey", 0.0f);
        }
        else
        {
            GAMEPLAY::_SET_CLOUD_HAT_OPACITY(1.0f);
            GAMEPLAY::LOAD_CLOUD_HAT("Snowy 01", 0.0f);
        }
        defaultRainFX = false;
        GAMEPLAY::_SET_RAIN_FX_INTENSITY(0.0f);
        if (!isDefaultWind)
        {
            GAMEPLAY::SET_WIND_SPEED(0.0f);
        }

        if (isAllModifiersDisabled())
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER(spaceZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
        }
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);

        doLightningInSpace();

        spaceZoneUpdate();

        sprintf_s(text[0], "spacezone\n");
    }

    const int currentWeatherIndex = static_cast<int>(currentWeatherType);
    const int aboveWeatherIndex   = static_cast<int>(aboveWeatherOverride[currentWeatherIndex]);
    const int spaceWeatherIndex   = static_cast<int>(spaceWeatherOverride[currentWeatherIndex]);

    sprintf_s(text[0], "%snormalZoneEnd: %.2f\ncloudZoneStart: %.2f\ncloudZoneEnd: %.2f", text[0], normalZoneEnd[currentWeatherIndex], cloudZoneStart[currentWeatherIndex], cloudZoneEnd[currentWeatherIndex]);
    sprintf_s(text[1], "aboveCloudsZoneStart: %.2f\naboveCloudsZoneEnd : %.2f\nspaceZoneStart : %.2f", aboveCloudsZoneStart[currentWeatherIndex], aboveCloudsZoneEnd[currentWeatherIndex], spaceZoneStart[currentWeatherIndex]);
    sprintf_s(text[2], "cloudZoneWindMultiplier: %.2f\naboveCloudsZoneWindMultiplier: %.2f\nspaceZoneWindMultiplier : %.2f", cloudWindMultiplier[currentWeatherIndex], aboveWindMultiplier[currentWeatherIndex], spaceWindMultiplier[currentWeatherIndex]);
    sprintf_s(text[3], "aboveCloudWeatherOverride: %s\nspaceZoneWeatherOverride: %s", weatherTypes[aboveWeatherIndex], weatherTypes[spaceWeatherIndex]);
    sprintf_s(text[4], "normalZoneTimecycMod: %s\ncloudZoneTimecycMod : %s\naboveCloudsZoneTimecycMod: %s", normalZoneTimecycleModifier[currentWeatherIndex], cloudZoneTimecycleModifier[currentWeatherIndex], aboveCloudsZoneTimecycleModifier[currentWeatherIndex]);
    sprintf_s(text[5], "spaceZoneTimecycMod: %s\n\nwind speed : %.2f\ntimecycle index: %d\n%s", spaceZoneTimecycleModifier[currentWeatherIndex], GAMEPLAY::GET_WIND_SPEED(), GRAPHICS::GET_TIMECYCLE_MODIFIER_INDEX(), nameFile);
}

__forceinline void loadWeatherSettings(char* fileName, Weather weatherType, char* sectionName)
{
    const int weatherIndex = static_cast<int>(weatherType);

    char sValue[MAX_NAME];
    Cloud clouds = Cloud::NoClouds;

    normalZoneEnd[weatherIndex] = GetPrivateProfileFloat(sectionName, "NormalZoneEnd", "800.0", fileName);
    cloudZoneStart[weatherIndex] = GetPrivateProfileFloat(sectionName, "CloudZoneStart", "900.0", fileName);
    cloudZoneEnd[weatherIndex] = GetPrivateProfileFloat(sectionName, "CloudZoneEnd", "1200.0", fileName);
    aboveCloudsZoneStart[weatherIndex] = GetPrivateProfileFloat(sectionName, "AboveCloudsZoneStart", "1400.0", fileName);
    aboveCloudsZoneEnd[weatherIndex] = GetPrivateProfileFloat(sectionName, "AboveCloudsZoneEnd", "2000.0", fileName);
    spaceZoneStart[weatherIndex] = GetPrivateProfileFloat(sectionName, "SpaceZoneStart", "3000.0", fileName);
    cloudWindMultiplier[weatherIndex] = GetPrivateProfileFloat(sectionName, "CloudZoneWindMultiplier", "2.0", fileName);
    aboveWindMultiplier[weatherIndex] = GetPrivateProfileFloat(sectionName, "AboveCloudsZoneWindMultiplier", "0.5", fileName);
    spaceWindMultiplier[weatherIndex] = GetPrivateProfileFloat(sectionName, "SpaceZoneWindMultiplier", "0.0", fileName);

    GetPrivateProfileString(sectionName, "NormalZoneClouds", "NO_CLOUDS", sValue, MAX_NAME, fileName);
    for (unsigned i = 0; i < static_cast<unsigned>(Cloud::Count); i++)
    {
        if (!strcmp(sValue, cloudCodes[i]))
        {
            clouds = static_cast<Cloud>(i);
            break;
        }
    }
    strcpy(weatherClouds[weatherIndex], cloudTypes[static_cast<int>(clouds)]);

    GetPrivateProfileString(sectionName, "AboveCloudsZoneWeatherOverride", "EXTRASUNNY", sValue, MAX_NAME, fileName);
    for (unsigned i = 0; i < static_cast<unsigned>(Weather::Count); i++)
    {
        if (!strcmp(weatherTypes[i], sValue))
        {
            aboveWeatherOverride[weatherIndex] = static_cast<Weather>(i);
            break;
        }
    }

    GetPrivateProfileString(sectionName, "SpaceZoneWeatherOverride", "HALLOWEEN", sValue, MAX_NAME, fileName);
    for (unsigned i = 0; i < static_cast<unsigned>(Weather::Count); i++)
    {
        if (!strcmp(weatherTypes[i], sValue))
        {
            spaceWeatherOverride[weatherIndex] = static_cast<Weather>(i);
            break;
        }
    }

    GetPrivateProfileString(sectionName, "NormalZoneTimecycleModifier", "nextgen", normalZoneTimecycleModifier[weatherIndex], MAX_NAME, fileName);
    GetPrivateProfileString(sectionName, "CloudZoneTimecycleModifier", "nextgen", cloudZoneTimecycleModifier[weatherIndex], MAX_NAME, fileName);
    GetPrivateProfileString(sectionName, "AboveCloudsZoneTimecycleModifier", "nextgen", aboveCloudsZoneTimecycleModifier[weatherIndex], MAX_NAME, fileName);
    GetPrivateProfileString(sectionName, "SpaceZoneTimecycleModifier", "nextgen", spaceZoneTimecycleModifier[weatherIndex], MAX_NAME, fileName);

    aboveCloudsZoneTimecycleTransitionTime[weatherIndex] = GetPrivateProfileFloat(sectionName, "AboveCloudsZoneTimecycleTransitionTime", "30.0", fileName);
    spaceZoneTimecycleTransitionTime[weatherIndex] = GetPrivateProfileFloat(sectionName, "SpaceZoneTimecycleTransitionTime", "30.0", fileName);

    enableLightningAboveClouds[weatherIndex] = GetPrivateProfileBool(sectionName, "EnableLightningAboveClouds", false, fileName);
    enableLightningInSpace[weatherIndex] = GetPrivateProfileBool(sectionName, "EnableLightningInSpace", false, fileName);

    lightningFrequency[weatherIndex] = GetPrivateProfileInt(sectionName, "LightningFrequency", 5, fileName);
}

bool loadConfigFromFile(char* fileName, bool isMainFile)
{
    char namePath[MAX_PATH];

    char sValue[MAX_NAME];

    strcpy(namePath, path);
    strcat(namePath, fileName);

    if (GetPrivateProfileBool("GLOBAL", "EnableAlternateConfigFile", false, namePath))
    {
        GetPrivateProfileString("GLOBAL", "AlternateConfigFile", "atmosphere.ini", sValue, MAX_NAME, namePath);
        if (strcmp(sValue, "atmosphere.ini") && isMainFile)
        {
            if (loadConfigFromFile(sValue, false))
                return true;
        }
    }
    isDefaultWind = GetPrivateProfileBool("GLOBAL", "DefaultWind", false, namePath);

    reloadKey = GetPrivateProfileHex("KEYS", "ReloadKey", "0x2E", namePath);
    onOffKey = GetPrivateProfileHex("KEYS", "OnOffKey", "0x2D", namePath);
    debugKey = GetPrivateProfileHex("KEYS", "DebugKey", "0x23", namePath);

    posX = GetPrivateProfileFloat("DEBUG", "PosX", "0.015", namePath);
    posY = GetPrivateProfileFloat("DEBUG", "PosY", "0.50", namePath);
    textScale = GetPrivateProfileFloat("DEBUG", "TextScale", "0.33", namePath);

    textColorRed = GetPrivateProfileInt("DEBUG", "TextColorRed", 255, namePath);
    textColorGreen = GetPrivateProfileInt("DEBUG", "TextColorGreen", 255, namePath);
    textColorBlue = GetPrivateProfileInt("DEBUG", "TextColorBlue", 0, namePath);

    loadWeatherSettings(namePath, Weather::Extrasunny, "EXTRASUNNY");
    loadWeatherSettings(namePath, Weather::Clear, "CLEAR");
    loadWeatherSettings(namePath, Weather::Clouds, "CLOUDS");
    loadWeatherSettings(namePath, Weather::Smog, "SMOG");
    loadWeatherSettings(namePath, Weather::Neutral, "NEUTRAL");
    loadWeatherSettings(namePath, Weather::Snowlight, "SNOWLIGHT");
    loadWeatherSettings(namePath, Weather::Foggy, "FOGGY");
    loadWeatherSettings(namePath, Weather::Overcast, "OVERCAST");
    loadWeatherSettings(namePath, Weather::Rain, "RAIN");
    loadWeatherSettings(namePath, Weather::Thunder, "THUNDER");
    loadWeatherSettings(namePath, Weather::Clearing, "CLEARING");
    loadWeatherSettings(namePath, Weather::Snow, "SNOW");
    loadWeatherSettings(namePath, Weather::Blizzard, "BLIZZARD");
    loadWeatherSettings(namePath, Weather::Xmas, "XMAS");
    loadWeatherSettings(namePath, Weather::Halloween, "HALLOWEEN");

    return true;
}

uintptr_t FindPattern(const char *pattern, const char *mask, const char* startAddress, size_t size)
{
    const char* address_end = startAddress + size;
    const auto mask_length = static_cast<size_t>(strlen(mask) - 1);

    for (size_t i = 0; startAddress < address_end; startAddress++)
    {
        if (*startAddress == pattern[i] || mask[i] == '?')
        {
            if (mask[i + 1] == '\0')
            {
                return reinterpret_cast<uintptr_t>(startAddress) - mask_length;
            }

            i++;
        }
        else
        {
            i = 0;
        }
    }

    return 0;
}

uintptr_t FindPattern(const char *pattern, const char *mask)
{
    MODULEINFO module = {};
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &module, sizeof(MODULEINFO));

    return FindPattern(pattern, mask, reinterpret_cast<const char *>(module.lpBaseOfDll), module.SizeOfImage);
}

void ScriptMain()
{
    GetModuleFileName(NULL, path, MAX_PATH);
    for (unsigned i = MAX_PATH - 1; i >= 0; --i)
    {
        if (path[i] == '\\')
        {
            path[i + 1] = '\0';
            break;
        }
    }
    uintptr_t address = FindPattern("\x40\x53\x48\x83\xEC\x20\x80\x3D\x00\x00\x00\x00\x00\x8B\xDA\x75\x29", "xxxxxxxx????xxxxx");

    if (address)
    {
        *(UINT8*)(address + *(int*)(address + 8) + 13) = 0;
    }

    register bool isOnMod = true;
    register bool isDebug = false;

    weatherHashes[static_cast<int>(Weather::Extrasunny)] = GAMEPLAY::GET_HASH_KEY("extrasunny");
    weatherHashes[static_cast<int>(Weather::Clear)] = GAMEPLAY::GET_HASH_KEY("clear");
    weatherHashes[static_cast<int>(Weather::Clouds)] = GAMEPLAY::GET_HASH_KEY("clouds");
    weatherHashes[static_cast<int>(Weather::Smog)] = GAMEPLAY::GET_HASH_KEY("smog");
    weatherHashes[static_cast<int>(Weather::Neutral)] = GAMEPLAY::GET_HASH_KEY("neutral");
    weatherHashes[static_cast<int>(Weather::Snowlight)] = GAMEPLAY::GET_HASH_KEY("snowlight");
    weatherHashes[static_cast<int>(Weather::Foggy)] = GAMEPLAY::GET_HASH_KEY("foggy");
    weatherHashes[static_cast<int>(Weather::Overcast)] = GAMEPLAY::GET_HASH_KEY("overcast");
    weatherHashes[static_cast<int>(Weather::Rain)] = GAMEPLAY::GET_HASH_KEY("rain");
    weatherHashes[static_cast<int>(Weather::Thunder)] = GAMEPLAY::GET_HASH_KEY("thunder");
    weatherHashes[static_cast<int>(Weather::Clearing)] = GAMEPLAY::GET_HASH_KEY("clearing");
    weatherHashes[static_cast<int>(Weather::Snow)] = GAMEPLAY::GET_HASH_KEY("snow");
    weatherHashes[static_cast<int>(Weather::Blizzard)] = GAMEPLAY::GET_HASH_KEY("blizzard");
    weatherHashes[static_cast<int>(Weather::Xmas)] = GAMEPLAY::GET_HASH_KEY("xmas");
    weatherHashes[static_cast<int>(Weather::Halloween)] = GAMEPLAY::GET_HASH_KEY("halloween");

    GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION(&weather0, &weather1, &progress);

    currentWeatherType = getWeatherType((progress < 0.5f) ? weather0 : weather1);

    loadConfigFromFile(nameFile, true);

    register Entity entity;

    while (true)
    {
        if (IsKeyJustUp(reloadKey) || reload)
        {
            GRAPHICS::CLEAR_TIMECYCLE_MODIFIER();
            loadConfigFromFile(nameFile, true);
            reload = false;
        }
        if (IsKeyJustUp(onOffKey) || onOff)
        {
            if (isOnMod)
            {
                if (!isNormalZone)
                {
                    UnloadScript();
                    normalZoneUpdate();
                }
                isOnMod = false;
            }
            else
            {
                isOnMod = true;
            }
            onOff = false;
        }
        if (IsKeyJustUp(debugKey) || debug)
        {
            isDebug = !isDebug;
            debug = false;
        }
        entity = PLAYER::PLAYER_PED_ID();
        getDecorators(entity);
        if (isOnMod)
        {
            if (isDebug)
            {
                update_status_text();
            }
            update(entity);
        }
        WAIT(0);
    }
}

void UnloadScript()
{
    GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
    GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(weather0, weather1, progress);
    GRAPHICS::SET_TIMECYCLE_MODIFIER(normalZoneTimecycleModifier[static_cast<int>(currentWeatherType)]);
    GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(0.0f);
    GRAPHICS::CLEAR_TIMECYCLE_MODIFIER();
    resetCloudHat();
}