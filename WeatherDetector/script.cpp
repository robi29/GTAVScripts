/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#include "..\Common\inc\script.h"
#include "..\inc\enbseries.h"
#include <stdint.h>
#include <windows.h>
#include <Shlwapi.h>
#include <psapi.h>
#pragma comment( lib, "shlwapi.lib" )
#pragma comment( lib, "psapi.lib" )

_ENBGetSDKVersion       enbGetSDKVersion       = nullptr;
_ENBGetVersion          enbGetVersion          = nullptr;
_ENBGetGameIdentifier   enbGetGameIdentifier   = nullptr;
_ENBSetCallbackFunction enbSetCallbackFunction = nullptr;
_ENBGetParameter        enbGetParameter        = nullptr;
_ENBSetParameter        enbSetParameter        = nullptr;

void WINAPI CallbackFunction( ENBCallbackType calltype );

//#include "utils.h"

/*#include <string>

std::string statusText;
DWORD statusTextDrawTicksMax;
bool statusTextGxtEntry;

char text[256];

void set_status_text(std::string str, DWORD time = 2500, bool isGxtEntry = false)
{
statusText = str;
statusTextDrawTicksMax = GetTickCount() + time;
statusTextGxtEntry = isGxtEntry;
}

void update_status_text()
{
if (GetTickCount() < statusTextDrawTicksMax)
{
UI::SET_TEXT_FONT(0);
UI::SET_TEXT_SCALE(0.55, 0.55);
UI::SET_TEXT_COLOUR(255, 255, 255, 255);
UI::SET_TEXT_WRAP(0.0, 1.0);
UI::SET_TEXT_CENTRE(1);
UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
if (statusTextGxtEntry)
{
UI::_SET_TEXT_ENTRY((char *)statusText.c_str());
}
else
{
UI::_SET_TEXT_ENTRY("STRING");
UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)statusText.c_str());
}
UI::_DRAW_TEXT(0.5, 0.5);
}
}*/

// clang-format off
#define EXTRASUNNY_HEATHAZE     0.9f
#define CLEAR_HEATHAZE          0.8f
#define NEUTRAL_HEATHAZE        1.0f
#define SMOG_HEATHAZE           0.5f
#define FOGGY_HEATHAZE          0.0f
#define OVERCAST_HEATHAZE       0.0f
#define CLOUDS_HEATHAZE         0.2f
#define CLEARING_HEATHAZE       0.0f
#define RAIN_HEATHAZE           0.0f
#define THUNDER_HEATHAZE        0.0f
#define SNOW_HEATHAZE           0.0f
#define BLIZZARD_HEATHAZE       0.0f
#define LIGHTSNOW_HEATHAZE      0.0f
#define XMAS_HEATHAZE           0.0f
#define HALLOWEEN_HEATHAZE      0.0f
#define NULL_HEATHAZE           0.0f

#define H0          0.0f
#define H1          0.0f
#define H2          0.0f
#define H3          0.0f
#define H4          0.0f
#define H5          0.0f
#define H6          0.0f
#define H7          0.0f
#define H8          0.0f
#define H9          0.0f
#define H10         0.5f
#define H11         1.0f
#define H12         1.0f
#define H13         1.0f
#define H14         1.0f
#define H15         1.0f
#define H16         1.0f
#define H17         1.0f
#define H18         0.7f
#define H19         0.3f
#define H20         0.0f
#define H21         0.0f
#define H22         0.0f
#define H23         0.0f
// clang-format on

enum class Weathers : uint32_t
{
    Extrasunny = 0,
    Clear,
    Clouds,
    Smog,
    Foggy,
    Overcast,
    Rain,
    Thunder,
    Clearing,
    Neutral,
    Snow,
    Blizzard,
    Snowlight,
    Xmas,
    Halloween,
    Count
};

Hash weatherHashes[(uint32_t) Weathers::Count];

static constexpr float weathers[16] =
{
    EXTRASUNNY_HEATHAZE,
    CLEAR_HEATHAZE,
    CLOUDS_HEATHAZE,
    SMOG_HEATHAZE,
    FOGGY_HEATHAZE,
    OVERCAST_HEATHAZE,
    RAIN_HEATHAZE,
    THUNDER_HEATHAZE,
    CLEARING_HEATHAZE,
    NEUTRAL_HEATHAZE,
    SNOW_HEATHAZE,
    BLIZZARD_HEATHAZE,
    LIGHTSNOW_HEATHAZE,
    XMAS_HEATHAZE,
    HALLOWEEN_HEATHAZE,
    NULL_HEATHAZE
};

static constexpr float hours[25] =
{
    H0,
    H1,
    H2,
    H3,
    H4,
    H5,
    H6,
    H7,
    H8,
    H9,
    H10,
    H11,
    H12,
    H13,
    H14,
    H15,
    H16,
    H17,
    H18,
    H19,
    H20,
    H21,
    H22,
    H23,
    H0
};

__forceinline void update()
{
    int32_t screen_w = 0;
    int32_t screen_h = 0;

    GRAPHICS::GET_SCREEN_RESOLUTION( &screen_w, &screen_h );

    const float width  = 1.0f / (float) screen_w;
    const float height = 1.0f / (float) screen_h;

    Any   weather0 = 0;
    Any   weather1 = 0;
    float progress = 0.0f;

    int32_t  pixelRed     = 0;
    int32_t  pixelGreen   = 0;
    int32_t  pixelBlue    = 0;
    uint32_t weatherIndex = 0;

    GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION( &weather0, &weather1, &progress );

    progress *= 255.0f;

    for( uint32_t i = 0; i < (uint32_t) Weathers::Count; ++i )
    {
        if( weather0 == weatherHashes[i] )
            weatherIndex = i;
    }

    weatherIndex <<= 4;

    pixelRed |= weatherIndex;

    for( uint32_t i = 0; i < (uint32_t) Weathers::Count; ++i )
    {
        if( weather1 == weatherHashes[i] )
            weatherIndex = i;
    }

    pixelRed |= weatherIndex;
    pixelGreen = static_cast<int>( progress );
    pixelBlue  = 255;

    GRAPHICS::DRAW_RECT( 0.0, 0.0, width, height, pixelRed, pixelGreen, pixelBlue, 255 );

    //sprintf_s(text, "%03d %03d %03d\n", pixelRed, pixelGreen, pixelBlue);
    //set_status_text(text);

    pixelRed   = TIME::GET_CLOCK_HOURS();
    pixelGreen = TIME::GET_CLOCK_MINUTES();
    pixelBlue  = TIME::GET_CLOCK_SECONDS();

    GRAPHICS::DRAW_RECT( 1.0f, 0.0, width, height, pixelRed, pixelGreen, pixelBlue, 255 );

    //Cam camera = CAM::GET_RENDERING_CAM();// { return invoke<Cam>(0x5234F9F10919EABA); } // 0x5234F9F10919EABA 0x0FCF4DF1
    Vector3 vec = CAM::_GET_GAMEPLAY_CAM_ROT( 2 );

    vec.x += 90.0f;
    vec.x *= 256.0f;

    int angle1 = static_cast<int>( vec.x ) / 256;
    int angle2 = static_cast<int>( vec.x ) % 256;

    if( angle1 > 255 )
    {
        angle1 = 255;
    }
    else if( angle1 < 0 )
    {
        angle1 = 0;
    }

    //Player player = PLAYER::GET_PLAYER_INDEX();
    Vector3 pos = ENTITY::GET_ENTITY_COORDS( PLAYER::PLAYER_PED_ID(), 1 );

    //static Cam GET_RENDERING_CAM() { return invoke<Cam>(0x5234F9F10919EABA); } // 0x5234F9F10919EABA 0x0FCF4DF1
    //Vector3 pos;
    //pos.x = 0.0f;
    //pos.y = 0.0f;
    //pos.z = 0.0f;
    //if (CAM::IS_GAMEPLAY_CAM_RENDERING())
    //{
    //pos = CAM::_GET_GAMEPLAY_CAM_COORDS();// CAM::GET_CAM_COORD(CAM::GET_RENDERING_CAM());// { return invoke<Vector3>(0xBAC038F7459AE5AE, cam); } // 0xBAC038F7459AE5AE 0x7C40F09C
    //}
    float pos1 = 0.0f, pos2 = 0.0f, pos3 = 0.0f, pos4 = 0.0f, pos5 = 0.0f;
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( pos.x, pos.y, pos.z + 1000.0f, &pos1, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( pos.x, pos.y + 50.0f, pos.z + 1000.0f, &pos2, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( pos.x + 50.0f, pos.y, pos.z + 1000.0f, &pos3, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( pos.x, pos.y - 50.0f, pos.z + 1000.0f, &pos4, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( pos.x - 50.0f, pos.y, pos.z + 1000.0f, &pos5, 0 );

    float max = pos1;
    if( pos1 < pos2 )
    {
        max = pos2;
    }
    if( pos2 < pos3 )
    {
        max = pos3;
    }
    if( pos3 < pos4 )
    {
        max = pos4;
    }
    if( pos4 < pos5 )
    {
        max = pos5;
    }

    if( pos1 <= 0.0f )
    {
        pos1 = max;
    }
    if( pos2 <= 0.0f )
    {
        pos2 = max;
    }
    if( pos3 <= 0.0f )
    {
        pos3 = max;
    }
    if( pos4 <= 0.0f )
    {
        pos4 = max;
    }
    if( pos5 <= 0.0f )
    {
        pos5 = max;
    }

    const float avg = ( pos1 + pos2 + pos3 + pos4 + pos5 ) / 5;

    int diff = abs( int( ( pos.z - avg ) * 10.0f ) );

    if( diff > 255 )
    {
        diff = 255;
    }

    diff = 255 - diff;

    pixelRed   = angle1;
    pixelGreen = angle2;
    pixelBlue  = diff;

    GRAPHICS::DRAW_RECT( 0.0f, 1.0f, width, height, pixelRed, pixelGreen, pixelBlue, 255 );

    vec.z += 180.0f;
    vec.z /= 360.0f;
    vec.z *= 256.0f;
    vec.z *= 256.0f;
    vec.z += 0.5f;

    pixelGreen = static_cast<int>( vec.z );

    pixelRed = pixelGreen / 256;
    pixelGreen %= 256;

    GRAPHICS::DRAW_RECT( 1.0f, 1.0f, width, height, pixelRed, pixelGreen, 0, 255 );

    //sprintf_s(text, "x = %f\nR = %d\nG = %d\nz = %f\nG = %d\nB = %d\nGB = %d", vec.x, angle, angle2, vec.z, R, G, R*256 + G);
    //set_status_text(text);
}

float lerp( const float v0, const float v1, const float t )
{
    return ( 1 - t ) * v0 + t * v1;
}

float ComputeFadeOutHeatHaze( const uint32_t currentWeather, const uint32_t nextWeather, const float progress, const uint32_t hour, const uint32_t minute, const uint32_t second )
{
    const float currentWeatherHaze = weathers[currentWeather];
    const float nextWeatherHaze    = weathers[nextWeather];
    const float weatherHazeAmount  = lerp( currentWeatherHaze, nextWeatherHaze, progress );
    const float hourHazeAmount     = lerp( hours[hour], hours[hour + 1], minute / 60.0f + second / 3600.0f );

    return weatherHazeAmount * hourHazeAmount;
}

void ExecuteSomething()
{
    char  shaderName[] = "ENBEFFECTPOSTPASS.FX";
    Any   weather0     = 0;
    Any   weather1     = 0;
    float progress     = 0;

    GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION( &weather0, &weather1, &progress );

    ENBParameter parameter;

    // Weathers.
    uint32_t currentWeather = 0;

    parameter.Type = ENBParam_INT;
    parameter.Size = ENBParameterTypeToSize( parameter.Type );

    for( uint32_t i = 0; i < (uint32_t) Weathers::Count; ++i )
    {
        if( weather0 == weatherHashes[i] )
        {
            currentWeather = i;
        }
    }

    memcpy( parameter.Data, &currentWeather, parameter.Size );

    char currentWeatherName[] = "Current Weather";
    enbSetParameter( nullptr, shaderName, currentWeatherName, &parameter );

    uint32_t nextWeather = 0;

    for( uint8_t i = 0; i < (uint8_t) Weathers::Count; ++i )
    {
        if( weather1 == weatherHashes[i] )
        {
            nextWeather = i;
        }
    }

    memcpy( parameter.Data, &nextWeather, parameter.Size );

    char nextWeatherName[] = "Next Weather";
    enbSetParameter( nullptr, shaderName, nextWeatherName, &parameter );

    // Time.
    parameter.Type = ENBParam_INT;
    parameter.Size = ENBParameterTypeToSize( parameter.Type );

    // Hour.
    const uint32_t hour = TIME::GET_CLOCK_HOURS();
    memcpy( parameter.Data, &hour, parameter.Size );

    char hourName[] = "Hour";
    enbSetParameter( nullptr, shaderName, hourName, &parameter );

    // Minute.
    const uint32_t minute = TIME::GET_CLOCK_MINUTES();
    memcpy( parameter.Data, &minute, parameter.Size );

    char minuteName[] = "Minute";
    enbSetParameter( nullptr, shaderName, minuteName, &parameter );

    // Second.
    const uint32_t second = TIME::GET_CLOCK_SECONDS();
    memcpy( parameter.Data, &second, parameter.Size );

    char secondName[] = "Second";
    enbSetParameter( nullptr, shaderName, secondName, &parameter );

    // Angle.
    ENBParameter parameterVector;
    parameterVector.Type = ENBParam_VECTOR3;
    parameterVector.Size = ENBParameterTypeToSize( parameterVector.Type );

    const Vector3 cameraRotation = CAM::_GET_GAMEPLAY_CAM_ROT( 2 );

    float vector[3];

    vector[0] = cameraRotation.x;
    vector[1] = -cameraRotation.x;
    vector[2] = -cameraRotation.z;

    vector[0] += 26.0f;
    vector[0] *= 0.019f;
    vector[0] -= 0.1f;

    vector[1] *= 0.164f;

    vector[2] *= 0.154f;

    memcpy( parameterVector.Data, &vector, parameterVector.Size );

    char angleName[] = "Angle";
    enbSetParameter( nullptr, shaderName, angleName, &parameterVector );

    // Progress.
    parameter.Type = ENBParam_FLOAT;
    parameter.Size = ENBParameterTypeToSize( parameter.Type );
    memcpy( parameter.Data, &progress, parameter.Size );

    char progressName[] = "Progress";
    enbSetParameter( nullptr, shaderName, progressName, &parameter );

    // Height.
    const Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS( PLAYER::PLAYER_PED_ID(), TRUE );

    float pos1 = 0.0f;
    float pos2 = 0.0f;
    float pos3 = 0.0f;
    float pos4 = 0.0f;
    float pos5 = 0.0f;

    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( playerPosition.x, playerPosition.y, playerPosition.z + 1000.0f, &pos1, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( playerPosition.x, playerPosition.y + 50.0f, playerPosition.z + 1000.0f, &pos2, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( playerPosition.x + 50.0f, playerPosition.y, playerPosition.z + 1000.0f, &pos3, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( playerPosition.x, playerPosition.y - 50.0f, playerPosition.z + 1000.0f, &pos4, 0 );
    GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD( playerPosition.x - 50.0f, playerPosition.y, playerPosition.z + 1000.0f, &pos5, 0 );

    float minimum = 10000.0f;

    if( pos1 >= 0.0f && pos1 < minimum )
    {
        minimum = pos1;
    }
    if( pos2 >= 0.0f && pos2 < minimum )
    {
        minimum = pos2;
    }
    if( pos3 >= 0.0f && pos3 < minimum )
    {
        minimum = pos3;
    }
    if( pos4 >= 0.0f && pos4 < minimum )
    {
        minimum = pos4;
    }
    if( pos5 >= 0.0f && pos5 < minimum )
    {
        minimum = pos5;
    }

    if( minimum < 0.0f )
    {
        minimum = 0.0f;
    }

    if( pos1 <= 0.0f )
    {
        pos1 = minimum;
    }
    if( pos2 <= 0.0f )
    {
        pos2 = minimum;
    }
    if( pos3 <= 0.0f )
    {
        pos3 = minimum;
    }
    if( pos4 <= 0.0f )
    {
        pos4 = minimum;
    }
    if( pos5 <= 0.0f )
    {
        pos5 = minimum;
    }

    const float avg = ( pos1 + pos2 + pos3 + pos4 + pos5 ) / 5.0f;

    const float diff = ( playerPosition.z - avg ) / 50.0f;

    const float height = ( diff > 1.0f ) ? 1.0f : diff;

    memcpy( parameter.Data, &height, parameter.Size );

    char heightName[] = "Height";
    enbSetParameter( nullptr, shaderName, heightName, &parameter );

    // FadeOut.
    const float fadeOut = ComputeFadeOutHeatHaze( currentWeather, nextWeather, progress, hour, minute, second );

    memcpy( parameter.Data, &fadeOut, parameter.Size );

    char fadeOutName[] = "Fade Out";
    enbSetParameter( nullptr, shaderName, fadeOutName, &parameter );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//save parameters before modifying them
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void SaveParameters()
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//restore previously saved parameters
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void RestoreParameters()
{
}

void WINAPI CallbackFunction( ENBCallbackType calltype )
{
    if( calltype == ENBCallback_PostLoad )
    {
        //TODO you need to save original values to restore them later
        SaveParameters();
    }

    if( calltype == ENBCallback_PreSave )
    {
        //TODO you need to restore original values, if modified by ENBSetParameter function (otherwise this plugin changes will be saved to configs)
        RestoreParameters();
    }

    //in this example just ignore everything, except end frame callback
    if( calltype == ENBCallback_EndFrame )
    {
        //TODO your code here for processing parameters
        ExecuteSomething();
    }
}

void ScriptMain()
{
    weatherHashes[(long long) Weathers::Extrasunny] = GAMEPLAY::GET_HASH_KEY( "extrasunny" );
    weatherHashes[(long long) Weathers::Clear]      = GAMEPLAY::GET_HASH_KEY( "clear" );
    weatherHashes[(long long) Weathers::Clouds]     = GAMEPLAY::GET_HASH_KEY( "clouds" );
    weatherHashes[(long long) Weathers::Smog]       = GAMEPLAY::GET_HASH_KEY( "smog" );
    weatherHashes[(long long) Weathers::Foggy]      = GAMEPLAY::GET_HASH_KEY( "foggy" );
    weatherHashes[(long long) Weathers::Overcast]   = GAMEPLAY::GET_HASH_KEY( "overcast" );
    weatherHashes[(long long) Weathers::Rain]       = GAMEPLAY::GET_HASH_KEY( "rain" );
    weatherHashes[(long long) Weathers::Thunder]    = GAMEPLAY::GET_HASH_KEY( "thunder" );
    weatherHashes[(long long) Weathers::Clearing]   = GAMEPLAY::GET_HASH_KEY( "clearing" );
    weatherHashes[(long long) Weathers::Neutral]    = GAMEPLAY::GET_HASH_KEY( "neutral" );
    weatherHashes[(long long) Weathers::Snow]       = GAMEPLAY::GET_HASH_KEY( "snow" );
    weatherHashes[(long long) Weathers::Blizzard]   = GAMEPLAY::GET_HASH_KEY( "blizzard" );
    weatherHashes[(long long) Weathers::Snowlight]  = GAMEPLAY::GET_HASH_KEY( "snowlight" );
    weatherHashes[(long long) Weathers::Xmas]       = GAMEPLAY::GET_HASH_KEY( "xmas" );
    weatherHashes[(long long) Weathers::Halloween]  = GAMEPLAY::GET_HASH_KEY( "halloween" );

    DWORD   cb             = 1000 * sizeof( HMODULE );
    DWORD   cbNeeded       = 0;
    HMODULE enbmodule      = nullptr;
    HMODULE hmodules[1000] = {};
    HANDLE  hproc          = GetCurrentProcess();
    //for (long i = 0; i < 1000; i++) hmodules[i] = NULL;

    //find proper library by existance of exported function, because several with the same name may exist
    if( EnumProcessModules( hproc, hmodules, cb, &cbNeeded ) )
    {
        long count = cbNeeded / sizeof( HMODULE );

        for( long i = 0; i < count; ++i )
        {
            if( hmodules[i] == nullptr )
            {
                break;
            }

            void* func = (void*) GetProcAddress( hmodules[i], "ENBGetSDKVersion" );

            if( func )
            {
                enbmodule = hmodules[i];
                break;
            }
        }
    }

    if( enbmodule )
    {
        enbGetSDKVersion       = (_ENBGetSDKVersion) GetProcAddress( enbmodule, "ENBGetSDKVersion" );
        enbGetVersion          = (_ENBGetVersion) GetProcAddress( enbmodule, "ENBGetVersion" );
        enbGetGameIdentifier   = (_ENBGetGameIdentifier) GetProcAddress( enbmodule, "ENBGetGameIdentifier" );
        enbSetCallbackFunction = (_ENBSetCallbackFunction) GetProcAddress( enbmodule, "ENBSetCallbackFunction" );
        enbGetParameter        = (_ENBGetParameter) GetProcAddress( enbmodule, "ENBGetParameter" );
        enbSetParameter        = (_ENBSetParameter) GetProcAddress( enbmodule, "ENBSetParameter" );
    }

    if( enbGetSDKVersion && enbGetVersion && enbGetGameIdentifier && enbSetCallbackFunction && enbGetParameter && enbSetParameter )
    {
        enbSetCallbackFunction( CallbackFunction );
    }
    else
    {
        //GRAPHICS::_DRAW_LIGHT_WITH_RANGE_WITH_SHADOW(-1559.07, -1168.37, 4.54, 255, 0, 0, 10.0, 1.0, 0.7);

        //GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW(-1559.07, -1148.37, 4.54, -1559.07, -1168.37, 4.54,
        //  0, 255, 0, 20.0, 1.0, 1.0, 90.0, 1.0, 0.7);
        bool reshadeMode = false;
        char path[MAX_PATH];

        if( GetModuleFileName( nullptr, path, MAX_PATH ) )
        {
            for( size_t i = strlen( path ); i > 0; --i )
            {
                if( path[i] == '\\' )
                {
                    path[i] = '\0';
                    break;
                }
            }

            strcat_s( path, "\\dxgi.dll" );

            if( PathFileExists( path ) )
            {
                reshadeMode = true;
            }
        }

        if( reshadeMode )
        {
            while( true )
            {
                update();
                //update_status_text();
                WAIT( 0 );
            }
        }
    }
}

void UnloadScript()
{
}
