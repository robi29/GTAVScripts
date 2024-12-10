#include "..\Common\inc\script.h"
#include "..\Common\inc\keyboard.h"

#include <string>
#include <unordered_map>
#include <algorithm>

unsigned int toggleKey       = 0x31;
unsigned int reloadKey       = 0x32;
bool         isOneShadowMode = false;
bool         isModEnabled    = true;

constexpr int vehsArraySize            = 300;
int           vehsArray[vehsArraySize] = {};

float brightness           = 2.0f;
float distance             = 25.0f;
float radius               = 50.0f;
float roundness            = 1.0f;
float falloff              = 20.0f;
float leftangle            = -0.7f;
float rightangle           = -0.6f;
float brightnessMultiplier = 1.5f;
float distanceMultiplier   = 2.0f;

struct Colour
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

constexpr Colour const id[] =
{
    { 0xFF, 0xFF, 0xCC }, // 0 (original 0)
    { 0x7F, 0xA7, 0xE3 }, // 1 (original 1)
    { 0xFF, 0xD8, 0x59 }, // 2 (original 2)
    { 0xFF, 0xE8, 0x94 }, // 3 (original 3)
    { 0xD1, 0x24, 0x24 }, // 4 (original 103)
    { 0xFF, 0xCE, 0x1C }, // 5 (original 87)
    { 0x24, 0xD1, 0x2C }, // 6 (original 76)
    { 0xFA, 0xEC, 0xC8 }, // 7 (original 14)
    { 0xFF, 0xEA, 0xA3 }, // 8 (original 181)
    { 0xFF, 0x00, 0x00 }  // 9 (for testing)
};

std::unordered_map<std::string, uint8_t>* vehLightID = nullptr;

__forceinline void update()
{
    const int vehicleArrayCount = worldGetAllVehicles( vehsArray, vehsArraySize );
    const int vehicleCount      = std::clamp( vehicleArrayCount, 0, vehsArraySize );

    const Entity  playerID        = PLAYER::PLAYER_PED_ID();
    const Vehicle playerVehicleId = PED::IS_PED_IN_ANY_VEHICLE( playerID, 0 )
        ? PED::GET_VEHICLE_PED_IS_USING( playerID )
        : -1;

    Vector3 playerPos = ENTITY::GET_ENTITY_COORDS( playerID, TRUE );
    playerPos.z += 5.0f;

    for( int i = 0; i < vehicleCount; ++i )
    {
        const int vehicleID = vehsArray[i];
        if( vehicleID > 0 && ENTITY::DOES_ENTITY_EXIST( vehicleID ) )
        {
            if( vehicleID == playerVehicleId )
            {
                VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER( vehicleID, 1.0f );
                continue;
            }
            BOOL          highbeamsOn = FALSE;
            BOOL          lightsOn    = FALSE;
            const BOOL    result      = VEHICLE::GET_VEHICLE_LIGHTS_STATE( vehicleID, &lightsOn, &highbeamsOn );
            const BOOL    isRunning   = VEHICLE::GET_IS_VEHICLE_ENGINE_RUNNING( vehicleID );
            const Vector3 vehiclePos  = ENTITY::GET_ENTITY_COORDS( vehicleID, TRUE );
            if( result == TRUE && lightsOn == TRUE && isRunning == TRUE && playerPos.z > vehiclePos.z )
            {
                const Vector3 angle = ENTITY::GET_ENTITY_FORWARD_VECTOR( vehicleID );

                float leftAngle  = angle.z;
                float rightAngle = angle.z;

                float lightDistance   = distance;
                float lightBrightness = brightness;

                if( highbeamsOn == TRUE )
                {
                    lightDistance   *= distanceMultiplier;
                    lightBrightness *= brightnessMultiplier;
                }
                else
                {
                    leftAngle  += leftangle;
                    rightAngle += rightangle;
                }
                VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER( vehicleID, 0.0f );

                const int left  = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME( vehicleID, const_cast<char*>( "headlight_l" ) );
                const int right = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME( vehicleID, const_cast<char*>( "headlight_r" ) );

                Vector3 leftLightPos  = { 0.0f, 0, 0.0f, 0, 0.0f, 0 };
                Vector3 rightLightPos = { 0.0f, 0, 0.0f, 0, 0.0f, 0 };

                if( left != -1 )
                {
                    leftLightPos = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE( vehicleID, left );
                }
                if( right != -1 )
                {
                    rightLightPos = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE( vehicleID, right );
                }

                const Hash  modelHash = ENTITY::GET_ENTITY_MODEL( vehicleID );
                std::string modelName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL( modelHash );

                std::transform( modelName.begin(), modelName.end(), modelName.begin(), ::toupper );

                const auto modelID = vehLightID->find( modelName );

                const int colorID = ( modelID != vehLightID->end() )
                    ? modelID->second % ( sizeof( id ) / sizeof( Colour ) )
                    : 0;
                    // : ( sizeof( id ) / sizeof( Colour ) ) - 1;

                const int red   = id[colorID].r;
                const int green = id[colorID].g;
                const int blue  = id[colorID].b;

                leftLightPos.x  += 0.22f * angle.x;
                leftLightPos.y  += 0.22f * angle.y;
                rightLightPos.x += 0.2f  * angle.x;
                rightLightPos.y += 0.2f  * angle.y;

                const BOOL isLeftDamaged  = VEHICLE::GET_IS_LEFT_VEHICLE_HEADLIGHT_DAMAGED( vehicleID );
                const BOOL isRightDamaged = VEHICLE::GET_IS_RIGHT_VEHICLE_HEADLIGHT_DAMAGED( vehicleID );

                if( isOneShadowMode )
                {
                    if( isLeftDamaged != TRUE || isRightDamaged != TRUE )
                    {
                        radius += 10.0f;
                        if( VEHICLE::IS_THIS_MODEL_A_BIKE( modelHash ) || VEHICLE::IS_THIS_MODEL_A_BICYCLE( modelHash ) )
                        {
                            leftLightPos.x = rightLightPos.x;
                            leftLightPos.y = rightLightPos.y;
                            leftLightPos.z = rightLightPos.z;
                        }
                        else
                        {
                            leftLightPos.x = ( leftLightPos.x + rightLightPos.x ) / 2;
                            leftLightPos.y = ( leftLightPos.y + rightLightPos.y ) / 2;
                            leftLightPos.z = ( leftLightPos.z + rightLightPos.z ) / 2;
                        }
                        GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW( leftLightPos.x, leftLightPos.y, leftLightPos.z, angle.x, angle.y, leftAngle, red, green, blue, lightDistance, lightBrightness, roundness, radius, falloff, vehicleID );
                    }
                }
                else
                {
                    if( isLeftDamaged != TRUE )
                    {
                        GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW( leftLightPos.x, leftLightPos.y, leftLightPos.z, angle.x, angle.y, leftAngle, red, green, blue, lightDistance, lightBrightness, roundness, radius, falloff, vehicleID );
                    }
                    if( isRightDamaged != TRUE )
                    {
                        GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW( rightLightPos.x, rightLightPos.y, rightLightPos.z, angle.x, angle.y, rightAngle, red, green, blue, lightDistance, lightBrightness, roundness, radius, falloff, vehicleID + 1 );
                    }
                }
            }
            else
            {
                VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER( vehicleID, 1.0f );
            }
        }
    }
}

unsigned GetPrivateProfileHex( const char* sectionName, const char* keyName, const char* defaultValue, const char* fileName )
{
    char sValue[32];
    GetPrivateProfileString( sectionName, keyName, defaultValue, sValue, 32, fileName );
    return strtol( sValue, nullptr, 16 );
}

float GetPrivateProfileFloat( const char* sectionName, const char* keyName, const char* defaultValue, const char* fileName )
{
    char sValue[32];
    GetPrivateProfileString( sectionName, keyName, defaultValue, sValue, 32, fileName );
    return strtof( sValue, nullptr );
}

bool GetPrivateProfileBool( const char* sectionName, const char* keyName, const bool defaultValue, const char* fileName )
{
    return ( GetPrivateProfileInt( sectionName, keyName, defaultValue ? 1 : 0, fileName ) == 1 ? true : false );
}

void loadConfigFromFile()
{
    char path[MAX_PATH];
    GetModuleFileName( nullptr, path, MAX_PATH );
    for( size_t i = strlen( path ); i > 0; --i )
    {
        if( path[i] == '\\' )
        {
            path[i] = '\0';
            break;
        }
    }
    strcat_s( path, "\\shadows.ini" );

    toggleKey = GetPrivateProfileHex( "KEYS", "ToggleKey", "0x31", path );
    reloadKey = GetPrivateProfileHex( "KEYS", "ReloadKey", "0x32", path );

    isOneShadowMode = GetPrivateProfileBool( "MODES", "OneShadowMode", false, path );

    brightness           = GetPrivateProfileFloat( "OPTIONS", "LightBrightness", "2.0", path );
    distance             = GetPrivateProfileFloat( "OPTIONS", "LightDistance", "25.0", path );
    radius               = GetPrivateProfileFloat( "OPTIONS", "LightRadius", "50.0", path );
    roundness            = GetPrivateProfileFloat( "OPTIONS", "LightRoundness", "1.0", path );
    falloff              = GetPrivateProfileFloat( "OPTIONS", "LightFallOff", "20.0", path );
    leftangle            = GetPrivateProfileFloat( "OPTIONS", "LeftLightAngle", "-0.7", path );
    rightangle           = GetPrivateProfileFloat( "OPTIONS", "RightLightAngle", "-0.6", path );
    brightnessMultiplier = GetPrivateProfileFloat( "OPTIONS", "HighBeamBrightnessMultiplier", "1.5", path );
    distanceMultiplier   = GetPrivateProfileFloat( "OPTIONS", "HighBeamDistanceMultiplier", "2.0", path );
}

void RestoreOriginalLightsSettings()
{
    const int vehicleArrayCount = worldGetAllVehicles( vehsArray, vehsArraySize );
    const int vehicleCount      = std::clamp( vehicleArrayCount, 0, vehsArraySize );

    for( int i = 0; i < vehicleCount; ++i )
    {
        const int vehicleID = vehsArray[i];
        if( vehicleID != 0 && ENTITY::DOES_ENTITY_EXIST( vehicleID ) )
        {
            VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER( vehicleID, 1.0f );
        }
    }
}

void ScriptMain()
{
    loadConfigFromFile();

    vehLightID = new( std::nothrow ) std::unordered_map<std::string, uint8_t>;

    if( vehLightID == nullptr )
    {
        return;
    }

    vehLightID->reserve( 1000 );

    vehLightID->try_emplace( "NINEF", 1 );
    vehLightID->try_emplace( "NINEF2", 1 );
    vehLightID->try_emplace( "ASEA", 0 );
    vehLightID->try_emplace( "BOXVILLE2", 0 );
    vehLightID->try_emplace( "BULLDOZE", 7 ); // BULLDOZER
    vehLightID->try_emplace( "CHEETAH", 1 );
    vehLightID->try_emplace( "COGCABRI", 1 ); // COGCABRIO
    vehLightID->try_emplace( "DUBSTA", 1 );
    vehLightID->try_emplace( "DUBSTA2", 0 );
    vehLightID->try_emplace( "EMPEROR", 0 );
    vehLightID->try_emplace( "ENTITYXF", 1 );
    vehLightID->try_emplace( "FIRETRUK", 0 );
    vehLightID->try_emplace( "FQ2", 0 );
    vehLightID->try_emplace( "INFERNUS", 1 );
    vehLightID->try_emplace( "JACKAL", 0 );
    vehLightID->try_emplace( "JOURNEY", 0 );
    vehLightID->try_emplace( "JB700", 0 );
    vehLightID->try_emplace( "ORACLE", 1 );
    vehLightID->try_emplace( "PATRIOT", 1 );
    vehLightID->try_emplace( "RADI", 1 );
    vehLightID->try_emplace( "ROMERO", 0 );
    vehLightID->try_emplace( "STINGER", 2 );
    vehLightID->try_emplace( "STOCKADE", 0 );
    vehLightID->try_emplace( "SUPERD", 1 );
    vehLightID->try_emplace( "TAILGATE", 1 ); // TAILGATER
    vehLightID->try_emplace( "TORNADO", 2 );
    vehLightID->try_emplace( "UTILTRUC", 0 ); // UTILLITRUCK
    vehLightID->try_emplace( "UTILLITRUCK2", 0 );
    vehLightID->try_emplace( "VOODOO2", 2 );
    vehLightID->try_emplace( "SCORCHER", 0 );
    vehLightID->try_emplace( "POLICEB", 1 );
    vehLightID->try_emplace( "HEXER", 1 );
    vehLightID->try_emplace( "BUZZARD", 0 );
    vehLightID->try_emplace( "POLMAV", 0 );
    vehLightID->try_emplace( "CUBAN800", 0 );
    vehLightID->try_emplace( "JET", 0 );
    vehLightID->try_emplace( "TITAN", 0 );
    vehLightID->try_emplace( "SQUALO", 0 );
    vehLightID->try_emplace( "MARQUIS", 0 );
    vehLightID->try_emplace( "FREIGHTCAR", 0 );
    vehLightID->try_emplace( "FREIGHT", 0 );
    vehLightID->try_emplace( "FREIGHTCONT1", 0 );
    vehLightID->try_emplace( "FREIGHTCONT2", 0 );
    vehLightID->try_emplace( "FREIGHTGRAIN", 0 );
    vehLightID->try_emplace( "TANKERCAR", 0 );
    vehLightID->try_emplace( "METROTRAIN", 0 );
    vehLightID->try_emplace( "TRAILERS", 0 );
    vehLightID->try_emplace( "TANKER", 0 );
    vehLightID->try_emplace( "TRAILERLOGS", 0 );
    vehLightID->try_emplace( "TR2", 0 );
    vehLightID->try_emplace( "TR3", 0 );
    vehLightID->try_emplace( "PICADOR", 0 );
    vehLightID->try_emplace( "POLICEO1", 2 ); // POLICEOLD1
    vehLightID->try_emplace( "POLICEO2", 2 ); // ADDED
    vehLightID->try_emplace( "ASTROPE", 0 );  // ASTEROPE
    vehLightID->try_emplace( "BANSHEE", 1 );
    vehLightID->try_emplace( "BUFFALO", 1 );
    vehLightID->try_emplace( "BULLET", 1 );
    vehLightID->try_emplace( "F620", 1 );
    vehLightID->try_emplace( "HANDLER", 7 );
    vehLightID->try_emplace( "RUINER", 2 );
    vehLightID->try_emplace( "GBURRITO", 0 );
    vehLightID->try_emplace( "TRACTOR2", 0 );
    vehLightID->try_emplace( "PENUMBRA", 0 );
    vehLightID->try_emplace( "SUBMERS", 3 ); // SUBMERSIBLE
    vehLightID->try_emplace( "DOCKTUG", 7 );
    vehLightID->try_emplace( "DOCKTRAILER", 0 );
    vehLightID->try_emplace( "SULTAN", 0 );
    vehLightID->try_emplace( "DILETTAN", 1 ); // DILETTANTE
    vehLightID->try_emplace( "FUTO", 2 );
    vehLightID->try_emplace( "HABANERO", 0 );
    vehLightID->try_emplace( "INTRUDER", 0 );
    vehLightID->try_emplace( "LANDSTAL", 1 ); // LANDSTALKER
    vehLightID->try_emplace( "MINIVAN", 0 );
    vehLightID->try_emplace( "SCHAFTER2", 1 );
    vehLightID->try_emplace( "SERRANO", 1 );
    vehLightID->try_emplace( "MANANA", 2 );
    vehLightID->try_emplace( "SEASHARK2", 0 );
    vehLightID->try_emplace( "YOUGA", 0 );
    vehLightID->try_emplace( "PREMIER", 1 );
    vehLightID->try_emplace( "SPEEDO", 0 );
    vehLightID->try_emplace( "WASHINGT", 0 ); // WASHINGTON
    vehLightID->try_emplace( "ANNIHILATOR", 0 );
    vehLightID->try_emplace( "BLAZER2", 0 );
    vehLightID->try_emplace( "CRUISER", 0 );
    vehLightID->try_emplace( "RAKETRAILER", 0 );
    vehLightID->try_emplace( "CARGOPLANE", 0 );
    vehLightID->try_emplace( "DUMP", 7 );
    vehLightID->try_emplace( "PONY", 2 );
    vehLightID->try_emplace( "LGUARD", 0 );
    vehLightID->try_emplace( "SENTINEL", 1 );
    vehLightID->try_emplace( "SENTINEL2", 1 );
    vehLightID->try_emplace( "COMET2", 1 );
    vehLightID->try_emplace( "STINGERG", 2 ); // STINGERGT
    vehLightID->try_emplace( "INGOT", 2 );
    vehLightID->try_emplace( "PEYOTE", 2 );
    vehLightID->try_emplace( "STANIER", 0 );
    vehLightID->try_emplace( "STRATUM", 2 );
    vehLightID->try_emplace( "AKUMA", 1 );
    vehLightID->try_emplace( "CARBON", 1 ); // ADDED
    vehLightID->try_emplace( "BATI", 1 );
    vehLightID->try_emplace( "BATI2", 1 );
    vehLightID->try_emplace( "PCJ", 0 );
    vehLightID->try_emplace( "DLOADER", 2 );
    vehLightID->try_emplace( "PRAIRIE", 1 );
    vehLightID->try_emplace( "DUSTER", 0 );
    vehLightID->try_emplace( "ISSI2", 1 );
    vehLightID->try_emplace( "TRAILERS2", 0 );
    vehLightID->try_emplace( "TVTRAILER", 0 );
    vehLightID->try_emplace( "CUTTER", 7 );
    vehLightID->try_emplace( "TRFLAT", 0 );
    vehLightID->try_emplace( "TORNADO2", 2 );
    vehLightID->try_emplace( "TORNADO3", 2 );
    vehLightID->try_emplace( "TRIBIKE", 0 );
    vehLightID->try_emplace( "TRIBIKE2", 0 );
    vehLightID->try_emplace( "TRIBIKE3", 0 );
    vehLightID->try_emplace( "PROPTRAILER", 0 );
    vehLightID->try_emplace( "BURRITO2", 0 );
    vehLightID->try_emplace( "DUNE", 0 );
    vehLightID->try_emplace( "FELTZER2", 1 );
    vehLightID->try_emplace( "BLISTA", 1 );
    vehLightID->try_emplace( "BAGGER", 0 );
    vehLightID->try_emplace( "VOLTIC", 1 );
    vehLightID->try_emplace( "FUGITIVE", 1 );
    vehLightID->try_emplace( "FELON", 1 );
    vehLightID->try_emplace( "PBUS", 0 );
    vehLightID->try_emplace( "ARMYTRAILER", 0 );
    vehLightID->try_emplace( "POLICET", 0 );
    vehLightID->try_emplace( "SPEEDO2", 2 );
    vehLightID->try_emplace( "FELON2", 1 );
    vehLightID->try_emplace( "BMX", 0 );
    vehLightID->try_emplace( "EXEMPLAR", 1 );
    vehLightID->try_emplace( "FUSILADE", 0 );
    vehLightID->try_emplace( "BOATTRAILER", 0 );
    vehLightID->try_emplace( "CAVCADE", 1 ); // CAVALCADE
    vehLightID->try_emplace( "SURGE", 0 );
    vehLightID->try_emplace( "BUCCANEE", 2 ); // BUCCANEER
    vehLightID->try_emplace( "NEMESIS", 1 );
    vehLightID->try_emplace( "ARMYTANKER", 0 );
    vehLightID->try_emplace( "ROCOTO", 1 );
    vehLightID->try_emplace( "STOCKADE3", 0 );
    vehLightID->try_emplace( "REBEL02", 2 );  // REBEL2
    vehLightID->try_emplace( "SCHWARZE", 1 ); // SCHWARZER
    vehLightID->try_emplace( "SCRAP", 7 );
    vehLightID->try_emplace( "SANDKING", 0 );
    vehLightID->try_emplace( "SANDKIN2", 0 ); // SANDKING2
    vehLightID->try_emplace( "CARBONIZ", 1 ); // CARBONIZZARE
    vehLightID->try_emplace( "RUMPO", 0 );
    vehLightID->try_emplace( "PRIMO", 0 );
    vehLightID->try_emplace( "SABREGT", 2 );
    vehLightID->try_emplace( "REGINA", 2 );
    vehLightID->try_emplace( "JETMAX", 0 );
    vehLightID->try_emplace( "TROPIC", 0 );
    vehLightID->try_emplace( "VIGERO", 0 );
    vehLightID->try_emplace( "POLICE2", 1 );
    vehLightID->try_emplace( "STRETCH", 1 );
    vehLightID->try_emplace( "DINGHY2", 0 );
    vehLightID->try_emplace( "BOXVILLE", 0 );
    vehLightID->try_emplace( "LUXOR", 0 );
    vehLightID->try_emplace( "POLICD3", 1 );
    vehLightID->try_emplace( "TRAILERS3", 0 );
    vehLightID->try_emplace( "DOUBLE", 1 );
    vehLightID->try_emplace( "TRACTOR", 7 );
    vehLightID->try_emplace( "BIFF", 7 );
    vehLightID->try_emplace( "DOMINATO", 0 ); // DOMINATOR
    vehLightID->try_emplace( "HAULER", 7 );
    vehLightID->try_emplace( "PACKER", 7 );
    vehLightID->try_emplace( "PHOENIX", 2 );
    vehLightID->try_emplace( "SADLER", 0 );
    vehLightID->try_emplace( "SADLER2", 0 );
    vehLightID->try_emplace( "DAEMON", 0 );
    vehLightID->try_emplace( "COACH", 0 );
    vehLightID->try_emplace( "TORNADO4", 2 );
    vehLightID->try_emplace( "RATLOADER", 2 );
    vehLightID->try_emplace( "RAPIDGT", 1 );
    vehLightID->try_emplace( "RAPIDGT2", 1 );
    vehLightID->try_emplace( "SURANO", 1 );
    vehLightID->try_emplace( "BFINJECT", 2 ); // BFINJECTION
    vehLightID->try_emplace( "BISON2", 0 );
    vehLightID->try_emplace( "BISON3", 0 );
    vehLightID->try_emplace( "BODHI2", 0 );
    vehLightID->try_emplace( "BURRITO", 0 );
    vehLightID->try_emplace( "BURRITO4", 0 );
    vehLightID->try_emplace( "RUBBLE", 7 );
    vehLightID->try_emplace( "TIPTRUCK", 7 );
    vehLightID->try_emplace( "TIPTRUCK2", 7 );
    vehLightID->try_emplace( "MIXER", 7 );
    vehLightID->try_emplace( "MIXER2", 7 );
    vehLightID->try_emplace( "PHANTOM", 7 );
    vehLightID->try_emplace( "POUNDER", 7 );
    vehLightID->try_emplace( "BUZZARD2", 0 );
    vehLightID->try_emplace( "FROGGER", 0 );
    vehLightID->try_emplace( "AIRTUG", 7 );
    vehLightID->try_emplace( "BENSON", 7 );
    vehLightID->try_emplace( "RIPLEY", 7 );
    vehLightID->try_emplace( "AMBULAN", 0 ); // AMBULANCE
    vehLightID->try_emplace( "FORK", 7 );    // FORKLIFT
    vehLightID->try_emplace( "GRANGER", 0 );
    vehLightID->try_emplace( "PRANGER", 0 );
    vehLightID->try_emplace( "TRAILERSMALL", 0 );
    vehLightID->try_emplace( "BARRACKS", 0 );
    vehLightID->try_emplace( "BARRACKS2", 0 );
    vehLightID->try_emplace( "CRUSADER", 0 );
    vehLightID->try_emplace( "UTILLITRUCK3", 0 );
    vehLightID->try_emplace( "SHERIFF", 0 );
    vehLightID->try_emplace( "MONROE", 2 );
    vehLightID->try_emplace( "MULE", 0 );
    vehLightID->try_emplace( "TACO", 0 );
    vehLightID->try_emplace( "TRASH", 7 );
    vehLightID->try_emplace( "DINGHY", 1 );
    vehLightID->try_emplace( "BLAZER", 0 );
    vehLightID->try_emplace( "MAVERICK", 0 );
    vehLightID->try_emplace( "CARGOBOB", 0 );
    vehLightID->try_emplace( "CARGOBOB3", 0 );
    vehLightID->try_emplace( "STUNT", 0 );
    vehLightID->try_emplace( "EMPEROR3", 2 );
    vehLightID->try_emplace( "CADDY", 0 );
    vehLightID->try_emplace( "EMPEROR2", 2 );
    vehLightID->try_emplace( "SURFER2", 2 );
    vehLightID->try_emplace( "TOWTRUCK", 7 );
    vehLightID->try_emplace( "TWOTRUCK2", 7 ); // TODO: typo?
    vehLightID->try_emplace( "BALLER", 1 );
    vehLightID->try_emplace( "SURFER", 2 );
    vehLightID->try_emplace( "MAMMATUS", 0 );
    vehLightID->try_emplace( "RIOT", 0 );
    vehLightID->try_emplace( "VELUM", 0 );
    vehLightID->try_emplace( "RANCHERX12", 0 );
    vehLightID->try_emplace( "CADDY2", 0 );
    vehLightID->try_emplace( "AIRBUS", 0 );
    vehLightID->try_emplace( "RENTBUS", 0 ); // RENTALBUS
    vehLightID->try_emplace( "GRESLEY", 1 );
    vehLightID->try_emplace( "ZION", 0 );
    vehLightID->try_emplace( "ZION2", 0 );
    vehLightID->try_emplace( "RUFFIAN", 1 );
    vehLightID->try_emplace( "ADDER", 1 );
    vehLightID->try_emplace( "VACCA", 1 );
    vehLightID->try_emplace( "BOXVILLE3", 0 );
    vehLightID->try_emplace( "SUNTRAP", 0 );
    vehLightID->try_emplace( "BOBCATXL", 2 );
    vehLightID->try_emplace( "BURRITO3", 0 );
    vehLightID->try_emplace( "POLICE4", 1 );
    vehLightID->try_emplace( "CABLECAR", 0 );
    vehLightID->try_emplace( "BLIMP", 0 );
    vehLightID->try_emplace( "BUS", 0 );
    vehLightID->try_emplace( "DILETTANTE2", 1 );
    vehLightID->try_emplace( "REBEL01", 0 ); // REBEL
    vehLightID->try_emplace( "SKYLIFT", 0 );
    vehLightID->try_emplace( "SHAMAL", 0 );
    vehLightID->try_emplace( "GRAINTRAILER", 0 );
    vehLightID->try_emplace( "VADER", 1 );
    vehLightID->try_emplace( "SHERIFF2", 0 );
    vehLightID->try_emplace( "BALETRAILER", 0 );
    vehLightID->try_emplace( "TOURBUS", 0 );
    vehLightID->try_emplace( "FIXTER", 0 );
    vehLightID->try_emplace( "ORACLE2", 1 );
    vehLightID->try_emplace( "BALLER2", 0 );
    vehLightID->try_emplace( "BUFFALO02", 0 ); // BUFFALO2
    vehLightID->try_emplace( "CAVALCADE2", 1 );
    vehLightID->try_emplace( "COQUETTE", 1 );
    vehLightID->try_emplace( "TRACTOR3", 2 );
    vehLightID->try_emplace( "GAUNTLET", 1 );
    vehLightID->try_emplace( "MESA", 0 );
    vehLightID->try_emplace( "POLICE", 1 );
    vehLightID->try_emplace( "CARGOBOB2", 0 );
    vehLightID->try_emplace( "TAXI", 0 );
    vehLightID->try_emplace( "SANCHEZ", 0 );
    vehLightID->try_emplace( "FLATBED", 0 );
    vehLightID->try_emplace( "SEMINOLE", 0 );
    vehLightID->try_emplace( "MOWER", 0 );
    vehLightID->try_emplace( "ZTYPE", 0 );
    vehLightID->try_emplace( "PREDATOR", 1 );
    vehLightID->try_emplace( "RUMPO2", 0 );
    vehLightID->try_emplace( "PONY2", 0 );
    vehLightID->try_emplace( "BJXL", 1 );
    vehLightID->try_emplace( "CAMPER", 0 );
    vehLightID->try_emplace( "RANCHERX", 0 ); // RANCHERXL
    vehLightID->try_emplace( "FAGGIO2", 0 );
    vehLightID->try_emplace( "LAZER", 0 );
    vehLightID->try_emplace( "SEASHARK", 0 );
    vehLightID->try_emplace( "BISON", 1 );
    vehLightID->try_emplace( "FBI", 1 );
    vehLightID->try_emplace( "FBI2", 0 );
    vehLightID->try_emplace( "MULE2", 0 );
    vehLightID->try_emplace( "RHINO", 0 );
    vehLightID->try_emplace( "BURRITO5", 0 );
    vehLightID->try_emplace( "ASES2", 2 );
    vehLightID->try_emplace( "MESA2", 0 );
    vehLightID->try_emplace( "MESA3", 0 );
    vehLightID->try_emplace( "FROGGER2", 0 );
    vehLightID->try_emplace( "HOTKNIFE", 1 );
    vehLightID->try_emplace( "ELEGY2", 1 );
    vehLightID->try_emplace( "KHAMEL", 1 ); // KHAMELION
    vehLightID->try_emplace( "DUNE2", 6 );
    vehLightID->try_emplace( "ARMYTRAILER2", 0 );
    vehLightID->try_emplace( "FREIGHTTRAILER", 0 );
    vehLightID->try_emplace( "TR4", 0 );
    vehLightID->try_emplace( "BLAZER03", 0 );  // BLAZER3
    vehLightID->try_emplace( "SANCHEZ01", 0 ); // ADDED
    vehLightID->try_emplace( "SANCHEZ02", 0 ); // SANCHEZ2
    vehLightID->try_emplace( "VERLIER", 1 );   // mpapartment // VERLIERER2
    vehLightID->try_emplace( "MAMBA", 2 );
    vehLightID->try_emplace( "NITESHAD", 2 ); // NIGHTSHADE
    vehLightID->try_emplace( "COG55", 1 );
    vehLightID->try_emplace( "COG552", 1 );
    vehLightID->try_emplace( "COGNOSC", 1 );  // COGNOSCENTI
    vehLightID->try_emplace( "COGNOSC2", 1 ); // COGNOSCENTI2
    vehLightID->try_emplace( "SCHAFTER3", 1 );
    vehLightID->try_emplace( "SCHAFTER4", 1 );
    vehLightID->try_emplace( "SCHAFTER5", 1 );
    vehLightID->try_emplace( "SCHAFTER6", 1 );
    vehLightID->try_emplace( "LIMO2", 1 );
    vehLightID->try_emplace( "BALLER3", 0 );
    vehLightID->try_emplace( "BALLER4", 0 );
    vehLightID->try_emplace( "BALLER5", 0 );
    vehLightID->try_emplace( "BALLER6", 0 );
    vehLightID->try_emplace( "SEASHARK3", 0 );
    vehLightID->try_emplace( "DINGHY4", 0 );
    vehLightID->try_emplace( "TROPIC2", 0 );
    vehLightID->try_emplace( "SPEEDER2", 0 );
    vehLightID->try_emplace( "TORO2", 0 );
    vehLightID->try_emplace( "CARGOBOB4", 0 );
    vehLightID->try_emplace( "SUPERVOLITO", 0 );
    vehLightID->try_emplace( "SUPERVOLITO2", 0 );
    vehLightID->try_emplace( "VALKYRIE2", 0 );
    vehLightID->try_emplace( "BIFTA", 0 ); // mpbeach
    vehLightID->try_emplace( "SPEEDER", 0 );
    vehLightID->try_emplace( "KALAHARI", 0 );
    vehLightID->try_emplace( "PARADISE", 0 );
    vehLightID->try_emplace( "TORNADO6", 2 ); // mpbiker
    vehLightID->try_emplace( "RAPTOR", 1 );
    vehLightID->try_emplace( "VORTEX", 1 );
    vehLightID->try_emplace( "AVARUS", 1 );
    vehLightID->try_emplace( "SANCTUS", 4 );
    vehLightID->try_emplace( "FAGGIO3", 0 );
    vehLightID->try_emplace( "FAGGIO", 0 );
    vehLightID->try_emplace( "FAGGION", 0 ); // ADDED
    vehLightID->try_emplace( "HAKUCHOU2", 1 );
    vehLightID->try_emplace( "NIGHTBLADE", 0 );
    vehLightID->try_emplace( "YOUGA2", 1 );
    vehLightID->try_emplace( "CHIMERA", 0 );
    vehLightID->try_emplace( "ESSKEY", 0 );
    vehLightID->try_emplace( "ZOMBIEA", 0 );
    vehLightID->try_emplace( "WOLFSBANE", 0 );
    vehLightID->try_emplace( "DAEMON2", 0 );
    vehLightID->try_emplace( "SHOTARO", 0 );
    vehLightID->try_emplace( "RATBIKE", 0 );
    vehLightID->try_emplace( "ZOMBIEB", 0 );
    vehLightID->try_emplace( "DEFILER", 0 );
    vehLightID->try_emplace( "MANCHEZ", 0 );
    vehLightID->try_emplace( "BLAZER4", 0 );
    vehLightID->try_emplace( "JESTER", 0 ); // mpbusiness
    vehLightID->try_emplace( "TURISMOR", 0 );
    vehLightID->try_emplace( "ALPHA", 1 );
    vehLightID->try_emplace( "VESTRA", 0 );
    vehLightID->try_emplace( "ZENTORNO", 0 ); // mpbusiness2
    vehLightID->try_emplace( "MASSACRO", 1 );
    vehLightID->try_emplace( "HUNTLEY", 1 );
    vehLightID->try_emplace( "THRUST", 1 );
    vehLightID->try_emplace( "SLAMVAN", 2 );  // mpchristmas2
    vehLightID->try_emplace( "RLOADER2", 2 ); // RATLOADER2
    vehLightID->try_emplace( "JESTER2", 0 );
    vehLightID->try_emplace( "MASSACRO2", 1 );
    vehLightID->try_emplace( "NIMBUS", 0 ); // mpexecutive
    vehLightID->try_emplace( "XLS", 1 );
    vehLightID->try_emplace( "XLS2", 1 );
    vehLightID->try_emplace( "SEVEN70", 1 );
    vehLightID->try_emplace( "REAPER", 1 );
    vehLightID->try_emplace( "FMJ", 1 );
    vehLightID->try_emplace( "PFISTER811", 1 );
    vehLightID->try_emplace( "BESTIAGTS", 1 );
    vehLightID->try_emplace( "BRICKADE", 0 );
    vehLightID->try_emplace( "RUMPO3", 0 );
    vehLightID->try_emplace( "PROTOTIPO", 1 );
    vehLightID->try_emplace( "WINDSOR2", 1 );
    vehLightID->try_emplace( "VOLATUS", 0 );
    vehLightID->try_emplace( "TUG", 0 );
    vehLightID->try_emplace( "TRAILERS4", 0 ); // mpgunrunning
    vehLightID->try_emplace( "XA21", 0 );
    vehLightID->try_emplace( "VAGNER", 1 );
    vehLightID->try_emplace( "CADDY3", 0 );
    vehLightID->try_emplace( "PHANTOM3", 1 );
    vehLightID->try_emplace( "NIGHTSHARK", 1 );
    vehLightID->try_emplace( "CHEETAH2", 1 );
    vehLightID->try_emplace( "TORERO", 1 );
    vehLightID->try_emplace( "HAULER2", 7 );
    vehLightID->try_emplace( "TRAILERLARGE", 1 );
    vehLightID->try_emplace( "TECHNICAL3", 0 );
    vehLightID->try_emplace( "TAMPA3", 2 );
    vehLightID->try_emplace( "INSURGENT3", 1 );
    vehLightID->try_emplace( "APC", 2 );
    vehLightID->try_emplace( "HALFTRACK", 2 );
    vehLightID->try_emplace( "DUNE3", 0 );
    vehLightID->try_emplace( "TRAILERSMALL2", 0 );
    vehLightID->try_emplace( "ARDENT", 1 );
    vehLightID->try_emplace( "OPPRESSOR", 0 );
    vehLightID->try_emplace( "LURCHER", 2 ); // mphalloween
    vehLightID->try_emplace( "BTYPE2", 2 );
    vehLightID->try_emplace( "MULE3", 0 ); // mpheist
    vehLightID->try_emplace( "TRASH2", 7 );
    vehLightID->try_emplace( "VELUM2", 0 );
    vehLightID->try_emplace( "TANKER2", 0 );
    vehLightID->try_emplace( "ENDURO", 1 );
    vehLightID->try_emplace( "SAVAGE", 1 );
    vehLightID->try_emplace( "CASCO", 0 );
    vehLightID->try_emplace( "TECHNICAL", 0 );
    vehLightID->try_emplace( "INSURGENT", 0 );
    vehLightID->try_emplace( "INSURGENT2", 0 );
    vehLightID->try_emplace( "HYDRA", 0 );
    vehLightID->try_emplace( "BOXVILLE4", 0 );
    vehLightID->try_emplace( "GBURRITO2", 0 );
    vehLightID->try_emplace( "GUARDIAN", 1 );
    vehLightID->try_emplace( "DINGHY3", 0 );
    vehLightID->try_emplace( "LECTRO", 1 );
    vehLightID->try_emplace( "KURUMA", 1 );
    vehLightID->try_emplace( "KURUMA2", 1 );
    vehLightID->try_emplace( "BARRCAKS3", 0 );
    vehLightID->try_emplace( "VALKYRIE", 0 );
    vehLightID->try_emplace( "SLAMVAN2", 0 );
    vehLightID->try_emplace( "RHAPSODY", 0 ); // mphipster
    vehLightID->try_emplace( "WARRENER", 2 );
    vehLightID->try_emplace( "BLADE", 2 );
    vehLightID->try_emplace( "GLENDALE", 5 );
    vehLightID->try_emplace( "PANTO", 0 );
    vehLightID->try_emplace( "DUBSTA3", 1 );
    vehLightID->try_emplace( "PIGALLE", 5 );
    vehLightID->try_emplace( "NERO", 0 ); // mpimportexport
    vehLightID->try_emplace( "NERO2", 0 );
    vehLightID->try_emplace( "ELEGY", 2 );
    vehLightID->try_emplace( "ITALIGTB", 0 );
    vehLightID->try_emplace( "ITALIGTB2", 0 );
    vehLightID->try_emplace( "TEMPESTA", 0 );
    vehLightID->try_emplace( "DIABLOUS", 1 );
    vehLightID->try_emplace( "SPECTER", 1 );
    vehLightID->try_emplace( "SPECTER2", 1 );
    vehLightID->try_emplace( "DIABLOUS2", 1 );
    vehLightID->try_emplace( "COMET3", 1 );
    vehLightID->try_emplace( "BLAZER5", 0 );
    vehLightID->try_emplace( "DUNE4", 1 );
    vehLightID->try_emplace( "DUNE5", 1 );
    vehLightID->try_emplace( "RUINER2", 2 );
    vehLightID->try_emplace( "VOLTIC2", 1 );
    vehLightID->try_emplace( "PHANTOM2", 1 );
    vehLightID->try_emplace( "BOXVILLE5", 1 );
    vehLightID->try_emplace( "WASTLNDR", 0 ); // WESTERLANDER
    vehLightID->try_emplace( "TECHNICAL2", 1 );
    vehLightID->try_emplace( "PENETRATOR", 1 );
    vehLightID->try_emplace( "FCR", 1 );
    vehLightID->try_emplace( "FCR2", 1 );
    vehLightID->try_emplace( "RUINER3", 1 );
    vehLightID->try_emplace( "MONSTER", 1 ); // mpindependence
    vehLightID->try_emplace( "SOVEREIGN", 1 );
    vehLightID->try_emplace( "SULTANRS", 1 ); // mpjanuary2016
    vehLightID->try_emplace( "BANSHEE2", 1 );
    vehLightID->try_emplace( "BUCCANEE2", 2 ); // mplowrider // BUCCANEER2
    vehLightID->try_emplace( "VOODOO", 2 );
    vehLightID->try_emplace( "FACTION", 2 );
    vehLightID->try_emplace( "FACTION2", 2 );
    vehLightID->try_emplace( "MOONBEAM", 2 );
    vehLightID->try_emplace( "MOONBEAM2", 2 );
    vehLightID->try_emplace( "PRIMO2", 2 );
    vehLightID->try_emplace( "CHINO2", 1 );
    vehLightID->try_emplace( "FACTION3", 2 ); // mplowrider2
    vehLightID->try_emplace( "MINIVAN2", 2 );
    vehLightID->try_emplace( "SABREGT2", 2 );
    vehLightID->try_emplace( "SLAMVAN3", 2 );
    vehLightID->try_emplace( "TORNADO5", 2 );
    vehLightID->try_emplace( "VIRGO2", 2 );
    vehLightID->try_emplace( "VIRGO3", 2 );
    vehLightID->try_emplace( "INNOVATION", 1 ); // mplts
    vehLightID->try_emplace( "HAKUCHOU", 1 );
    vehLightID->try_emplace( "FURORE", 0 ); // FUROREGT
    vehLightID->try_emplace( "SWIFT2", 0 ); // mpluxe
    vehLightID->try_emplace( "LUXOR2", 0 );
    vehLightID->try_emplace( "FELTZER3", 2 );
    vehLightID->try_emplace( "OSIRIS", 1 );
    vehLightID->try_emplace( "VIRGO", 2 );
    vehLightID->try_emplace( "WINDSOR", 1 );
    vehLightID->try_emplace( "COQUETTE3", 2 ); // mpluxe2
    vehLightID->try_emplace( "VINDICATOR", 0 );
    vehLightID->try_emplace( "T20", 0 );
    vehLightID->try_emplace( "BRAWLER", 1 );
    vehLightID->try_emplace( "TORO", 1 );
    vehLightID->try_emplace( "CHINO", 1 );
    vehLightID->try_emplace( "MILJET", 0 ); // mppilot
    vehLightID->try_emplace( "BESRA", 1 );
    vehLightID->try_emplace( "COQUETTE2", 2 );
    vehLightID->try_emplace( "SWIFT", 0 );
    vehLightID->try_emplace( "VIGILANTE", 1 ); // mpsmuggler
    vehLightID->try_emplace( "BOMBUSHKA", 1 );
    vehLightID->try_emplace( "HOWARD", 1 );
    vehLightID->try_emplace( "ALPHAZ1", 1 );
    vehLightID->try_emplace( "SEABREEZE", 1 );
    vehLightID->try_emplace( "NOKOTA", 1 );
    vehLightID->try_emplace( "MOLOTOK", 1 );
    vehLightID->try_emplace( "STARLING", 1 );
    vehLightID->try_emplace( "HAVOK", 1 );
    vehLightID->try_emplace( "TULA", 1 );
    vehLightID->try_emplace( "MICROLIGHT", 1 );
    vehLightID->try_emplace( "HUNTER", 1 );
    vehLightID->try_emplace( "ROGUE", 1 );
    vehLightID->try_emplace( "PYRO", 1 );
    vehLightID->try_emplace( "RAPIDGT3", 2 );
    vehLightID->try_emplace( "MOGUL", 1 );
    vehLightID->try_emplace( "RETINUE", 2 );
    vehLightID->try_emplace( "CYCLONE", 1 );
    vehLightID->try_emplace( "VISIONE", 1 );
    vehLightID->try_emplace( "TURISMO2", 1 ); // mpspecialraces
    vehLightID->try_emplace( "INFERNUS2", 1 );
    vehLightID->try_emplace( "GP1", 1 );
    vehLightID->try_emplace( "RUSTON", 1 );
    vehLightID->try_emplace( "GARGOYLE", 0 ); // mpstunt
    vehLightID->try_emplace( "OMNIS", 0 );
    vehLightID->try_emplace( "SHEAVA", 0 );
    vehLightID->try_emplace( "TYRUS", 0 );
    vehLightID->try_emplace( "LE7B", 0 );
    vehLightID->try_emplace( "LYNX", 0 );
    vehLightID->try_emplace( "TROPOS", 2 );
    vehLightID->try_emplace( "TAMPA2", 2 );
    vehLightID->try_emplace( "BRIOSO", 0 );
    vehLightID->try_emplace( "BF400", 1 );
    vehLightID->try_emplace( "CLIFFHANGER", 1 );
    vehLightID->try_emplace( "CONTENDER", 0 );
    vehLightID->try_emplace( "E109", 0 );    // ADDED
    vehLightID->try_emplace( "TROPHY", 1 );  // TROPHYTRUCK
    vehLightID->try_emplace( "TROPHY2", 1 ); // TROPHYTRUCK2
    vehLightID->try_emplace( "RALLYTRUCK", 0 );
    vehLightID->try_emplace( "ROOSEVELT", 0 );  // mpvalentines // BTYPE
    vehLightID->try_emplace( "ROOSEVELT2", 2 ); // mpvalentines2 // BTYPE3
    vehLightID->try_emplace( "TAMPA", 2 );      // mpxmas_604490
    vehLightID->try_emplace( "SUBMERS2", 3 );   // spupgrade // SUBMERSIBLE2
    vehLightID->try_emplace( "MARSHALL", 1 );
    vehLightID->try_emplace( "BLIMP2", 0 );
    vehLightID->try_emplace( "DUKES", 2 );
    vehLightID->try_emplace( "DUKES2", 2 );
    vehLightID->try_emplace( "BUFFALO3", 0 );
    vehLightID->try_emplace( "DOMINATO2", 0 ); // DOMINATOR2
    vehLightID->try_emplace( "GAUNTLET2", 1 );
    vehLightID->try_emplace( "STALION", 2 );
    vehLightID->try_emplace( "STALION2", 2 );
    vehLightID->try_emplace( "BLISTA2", 2 );
    vehLightID->try_emplace( "BLISTA3", 2 );
    vehLightID->try_emplace( "ADMIRAL", 0 ); // IV Pack
    vehLightID->try_emplace( "ANGEL", 0 );
    vehLightID->try_emplace( "APC2", 0 );
    vehLightID->try_emplace( "BLADE2", 1 );
    vehLightID->try_emplace( "BOBCAT", 2 );
    vehLightID->try_emplace( "BODHI", 0 );
    vehLightID->try_emplace( "BOXVILLE7", 0 );
    vehLightID->try_emplace( "BRICKADE3", 0 );
    vehLightID->try_emplace( "BUCCANEER3", 0 );
    vehLightID->try_emplace( "BUS2", 0 );
    vehLightID->try_emplace( "CABBY", 0 );
    vehLightID->try_emplace( "CHAVOS", 0 );
    vehLightID->try_emplace( "CHAVOS2", 0 );
    vehLightID->try_emplace( "CHEETAH3", 1 );
    vehLightID->try_emplace( "CONTENDER2", 0 );
    vehLightID->try_emplace( "COQUETTE7", 0 );
    vehLightID->try_emplace( "DF8", 1 );
    vehLightID->try_emplace( "DIABOLUS", 0 );
    vehLightID->try_emplace( "DOUBLE2", 1 );
    vehLightID->try_emplace( "ESPERANTO", 0 );
    vehLightID->try_emplace( "EMPEROR4", 0 );
    vehLightID->try_emplace( "FELTZER", 1 );
    vehLightID->try_emplace( "FEROCI", 0 );
    vehLightID->try_emplace( "FEROCI2", 0 );
    vehLightID->try_emplace( "FLATBED2", 0 );
    vehLightID->try_emplace( "FLOATER", 1 );
    vehLightID->try_emplace( "FORTUNE", 0 );
    vehLightID->try_emplace( "FREEWAY", 0 );
    vehLightID->try_emplace( "FUTO3", 2 );
    vehLightID->try_emplace( "FXT", 1 );
    vehLightID->try_emplace( "GHAWAR", 0 );
    vehLightID->try_emplace( "HAKUMAI", 0 );
    vehLightID->try_emplace( "HAKUCHOU3", 0 );
    vehLightID->try_emplace( "HELLFURY", 0 );
    vehLightID->try_emplace( "HUNTLEY2", 0 );
    vehLightID->try_emplace( "INTERC", 1 ); // INTERCEPTOR
    vehLightID->try_emplace( "JB7003", 2 );
    vehLightID->try_emplace( "LOKUS", 0 );
    vehLightID->try_emplace( "LYCAN", 0 );
    vehLightID->try_emplace( "LYCAN2", 0 );
    vehLightID->try_emplace( "MARBELLE", 2 );
    vehLightID->try_emplace( "MERIT", 0 );
    vehLightID->try_emplace( "MRTASTY", 0 );
    vehLightID->try_emplace( "NIGHTBLADE2", 0 );
    vehLightID->try_emplace( "NOOSE", 0 );
    vehLightID->try_emplace( "NRG900", 0 );
    vehLightID->try_emplace( "NSTOCKADE", 0 );
    vehLightID->try_emplace( "PACKER2", 0 );
    vehLightID->try_emplace( "PERENNIAL", 0 );
    vehLightID->try_emplace( "PERENNIAL2", 0 );
    vehLightID->try_emplace( "PHOENIX2", 2 );
    vehLightID->try_emplace( "PINNACLE", 0 );
    vehLightID->try_emplace( "PMP600", 0 );
    vehLightID->try_emplace( "POLICE6", 0 );
    vehLightID->try_emplace( "POLICE7", 1 );
    vehLightID->try_emplace( "POLICE8", 0 );
    vehLightID->try_emplace( "POLPATRIOT", 1 );
    vehLightID->try_emplace( "PREMIER2", 0 );
    vehLightID->try_emplace( "PRES", 1 );
    vehLightID->try_emplace( "PRES2", 1 );
    vehLightID->try_emplace( "PSTOCKADE", 0 );
    vehLightID->try_emplace( "RANCHER", 0 );
    vehLightID->try_emplace( "REBLA2", 3 );
    vehLightID->try_emplace( "REEFER", 3 );
    vehLightID->try_emplace( "REGINA2", 0 );
    vehLightID->try_emplace( "REGINA3", 2 );
    vehLightID->try_emplace( "REVENANT", 0 );
    vehLightID->try_emplace( "ROM", 0 );
    vehLightID->try_emplace( "SABRE", 2 );
    vehLightID->try_emplace( "SABRE2", 2 );
    vehLightID->try_emplace( "SCHAFTER", 1 );
    vehLightID->try_emplace( "SCHAFTERGTR", 1 );
    vehLightID->try_emplace( "SENTINEL5", 0 );
    vehLightID->try_emplace( "SMUGGLER", 1 );
    vehLightID->try_emplace( "SOLAIR", 0 );
    vehLightID->try_emplace( "SOVEREIGN2", 0 );
    vehLightID->try_emplace( "STANIER2", 0 );
    vehLightID->try_emplace( "STEED", 0 );
    vehLightID->try_emplace( "STRATUM2", 2 );
    vehLightID->try_emplace( "STRETCH2", 0 );
    vehLightID->try_emplace( "STRETCH3", 1 );
    vehLightID->try_emplace( "SULTAN4", 0 );
    vehLightID->try_emplace( "SUPERD2", 1 );
    vehLightID->try_emplace( "SUPERGT", 0 );
    vehLightID->try_emplace( "TAXI2", 2 );
    vehLightID->try_emplace( "TAXI3", 0 );
    vehLightID->try_emplace( "TOURMAV", 0 );
    vehLightID->try_emplace( "TURISMO", 0 );
    vehLightID->try_emplace( "TYPHOON", 0 );
    vehLightID->try_emplace( "URANUS", 0 );
    vehLightID->try_emplace( "VIGERO4", 2 );
    vehLightID->try_emplace( "VINCENT", 0 );
    vehLightID->try_emplace( "VIOLATOR", 1 );
    vehLightID->try_emplace( "VOODOO3", 2 );
    vehLightID->try_emplace( "WAYFARER", 0 );
    vehLightID->try_emplace( "WILLARD", 0 );
    vehLightID->try_emplace( "WOLFSBANE2", 0 );
    vehLightID->try_emplace( "YANKEE", 7 );
    vehLightID->try_emplace( "TANKEE2", 7 ); // TODO: typo?
    vehLightID->try_emplace( "COMET5", 1 ); // mpchristmas2017
    vehLightID->try_emplace( "RAIDEN", 1 );
    vehLightID->try_emplace( "VISERIS", 0 );
    vehLightID->try_emplace( "RIATA", 1 );
    vehLightID->try_emplace( "KAMACHO", 1 );
    vehLightID->try_emplace( "SC1", 1 );
    vehLightID->try_emplace( "AUTARCH", 1 );
    vehLightID->try_emplace( "SAVESTRA", 1 );
    vehLightID->try_emplace( "GT500", 0 );
    vehLightID->try_emplace( "NEON", 1 );
    vehLightID->try_emplace( "YOSEMITE", 2 );
    vehLightID->try_emplace( "HERMES", 2 );
    vehLightID->try_emplace( "HUSTLER", 2 );
    vehLightID->try_emplace( "SENTINEL3", 2 );
    vehLightID->try_emplace( "Z190", 1 );
    vehLightID->try_emplace( "KHANJALI", 1 );
    vehLightID->try_emplace( "BARRAGE", 1 );
    vehLightID->try_emplace( "VOLATOL", 2 );
    vehLightID->try_emplace( "AKULA", 1 );
    vehLightID->try_emplace( "AVENGER", 1 );
    vehLightID->try_emplace( "AVENGER2", 1 );
    vehLightID->try_emplace( "DELUXO", 1 );
    vehLightID->try_emplace( "STROMBERG", 1 );
    vehLightID->try_emplace( "CHERNOBOG", 2 );
    vehLightID->try_emplace( "RIOT2", 1 );
    vehLightID->try_emplace( "THRUSTER", 1 );
    vehLightID->try_emplace( "STREITER", 1 );
    vehLightID->try_emplace( "REVOLTER", 1 );
    vehLightID->try_emplace( "PARIAH", 0 );
    vehLightID->try_emplace( "CARACARA", 1 ); // mpassault
    vehLightID->try_emplace( "SEASPARROW", 1 );
    vehLightID->try_emplace( "ENTITY2", 1 );
    vehLightID->try_emplace( "JESTER3", 1 );
    vehLightID->try_emplace( "TYRANT", 1 );
    vehLightID->try_emplace( "DOMINATOR3", 1 );
    vehLightID->try_emplace( "HOTRING", 1 );
    vehLightID->try_emplace( "FLASHGT", 1 );
    vehLightID->try_emplace( "TEZERACT", 1 );
    vehLightID->try_emplace( "ELLIE", 1 );
    vehLightID->try_emplace( "MICHELLI", 1 );
    vehLightID->try_emplace( "GB200", 1 );
    vehLightID->try_emplace( "ISSI3", 1 );
    vehLightID->try_emplace( "TAIPAN", 0 );
    vehLightID->try_emplace( "FAGALOA", 2 );
    vehLightID->try_emplace( "CHEBUREK", 2 );
    vehLightID->try_emplace( "STAFFORD", 2 ); // mpbattle
    vehLightID->try_emplace( "SCRAMJET", 1 );
    vehLightID->try_emplace( "STRIKEFORCE", 0 );
    vehLightID->try_emplace( "TERBYTE", 1 );
    vehLightID->try_emplace( "PBUS2", 0 );
    vehLightID->try_emplace( "POUNDER2", 7 );
    vehLightID->try_emplace( "FREECRAWLER", 1 );
    vehLightID->try_emplace( "MULE4", 1 );
    vehLightID->try_emplace( "BLIMP3", 0 );
    vehLightID->try_emplace( "MENACER", 1 );
    vehLightID->try_emplace( "SWINGER", 0 );
    vehLightID->try_emplace( "PATRIOT2", 1 );
    vehLightID->try_emplace( "OPPRESSOR2", 0 );
    vehLightID->try_emplace( "IMPALER3", 2 ); // mpchristmas2018
    vehLightID->try_emplace( "MONSTER5", 1 );
    vehLightID->try_emplace( "SLAMVAN6", 1 );
    vehLightID->try_emplace( "ISSI6", 1 );
    vehLightID->try_emplace( "CERBERUS3", 1 );
    vehLightID->try_emplace( "DEATHBIKE2", 0 );
    vehLightID->try_emplace( "DOMINATOR6", 1 );
    vehLightID->try_emplace( "DEATHBIKE3", 1 );
    vehLightID->try_emplace( "IMPALER4", 2 );
    vehLightID->try_emplace( "SLAMVAN4", 1 );
    vehLightID->try_emplace( "SLAMVAN5", 1 );
    vehLightID->try_emplace( "BRUTUS3", 1 );
    vehLightID->try_emplace( "BRUTUS2", 1 );
    vehLightID->try_emplace( "BRUTUS", 1 );
    vehLightID->try_emplace( "DEATHBIKE", 1 );
    vehLightID->try_emplace( "BRUISER", 0 );
    vehLightID->try_emplace( "BRUISER2", 0 );
    vehLightID->try_emplace( "BRUISER3", 0 );
    vehLightID->try_emplace( "RCBANDITO", 1 );
    vehLightID->try_emplace( "CERBERUS", 1 );
    vehLightID->try_emplace( "CERBERUS2", 1 );
    vehLightID->try_emplace( "IMPALER2", 2 );
    vehLightID->try_emplace( "MONSTER4", 1 );
    vehLightID->try_emplace( "MONSTER3", 1 );
    vehLightID->try_emplace( "TULIP", 1 );
    vehLightID->try_emplace( "ITALIGTO", 1 );
    vehLightID->try_emplace( "ISSI4", 1 );
    vehLightID->try_emplace( "ISSI5", 1 );
    vehLightID->try_emplace( "SCARAB", 1 );
    vehLightID->try_emplace( "SCARAB2", 1 );
    vehLightID->try_emplace( "SCARAB3", 1 );
    vehLightID->try_emplace( "CLIQUE", 1 );
    vehLightID->try_emplace( "IMPALER", 2 );
    vehLightID->try_emplace( "VAMOS", 1 );
    vehLightID->try_emplace( "IMPERATOR", 2 );
    vehLightID->try_emplace( "IMPERATOR2", 2 );
    vehLightID->try_emplace( "IMPERATOR3", 2 );
    vehLightID->try_emplace( "TOROS", 1 );
    vehLightID->try_emplace( "SCHLAGEN", 1 );
    vehLightID->try_emplace( "DEVIANT", 1 );
    vehLightID->try_emplace( "DOMINATOR4", 1 );
    vehLightID->try_emplace( "DOMINATOR5", 1 );
    vehLightID->try_emplace( "ZR380", 1 );
    vehLightID->try_emplace( "ZR3802", 1 );
    vehLightID->try_emplace( "ZR3803", 1 );
    vehLightID->try_emplace( "DEVESTE", 1 );
    vehLightID->try_emplace( "PARAGON", 1 ); // mpvinewood
    vehLightID->try_emplace( "PARAGON2", 1 );
    vehLightID->try_emplace( "JUGULAR", 1 );
    vehLightID->try_emplace( "RROCKET", 0 );
    vehLightID->try_emplace( "PEYOTE2", 2 );
    vehLightID->try_emplace( "NEO", 1 );
    vehLightID->try_emplace( "KRIEGER", 1 );
    vehLightID->try_emplace( "S80", 1 );
    vehLightID->try_emplace( "DYNASTY", 0 );
    vehLightID->try_emplace( "CARACARA2", 1 );
    vehLightID->try_emplace( "THRAX", 1 );
    vehLightID->try_emplace( "NOVAK", 1 );
    vehLightID->try_emplace( "ZORRUSSO", 1 );
    vehLightID->try_emplace( "ISSI7", 1 );
    vehLightID->try_emplace( "LOCUST", 1 );
    vehLightID->try_emplace( "EMERUS", 1 );
    vehLightID->try_emplace( "HELLION", 1 );
    vehLightID->try_emplace( "GAUNTLET3", 2 );
    vehLightID->try_emplace( "GAUNTLET4", 1 );
    vehLightID->try_emplace( "NEBULA", 2 );
    vehLightID->try_emplace( "ZION3", 2 );
    vehLightID->try_emplace( "DRAFTER", 1 );
    vehLightID->try_emplace( "RETINUE2", 2 ); // mpheist3
    vehLightID->try_emplace( "OUTLAW", 1 );
    vehLightID->try_emplace( "STRYDER", 0 );
    vehLightID->try_emplace( "YOSEMITE2", 2 );
    vehLightID->try_emplace( "FORMULA", 0 );
    vehLightID->try_emplace( "FORMULA2", 0 );
    vehLightID->try_emplace( "MINITANK", 0 );
    vehLightID->try_emplace( "SUGOI", 0 );
    vehLightID->try_emplace( "SULTAN2", 2 );
    vehLightID->try_emplace( "EVERON", 0 );
    vehLightID->try_emplace( "REBLA", 0 );
    vehLightID->try_emplace( "VAGRANT", 1 );
    vehLightID->try_emplace( "FURIA", 0 );
    vehLightID->try_emplace( "VSTR", 0 );
    vehLightID->try_emplace( "KOMODA", 0 );
    vehLightID->try_emplace( "ASBO", 2 );
    vehLightID->try_emplace( "ZHABA", 1 );
    vehLightID->try_emplace( "JB7002", 1 );
    vehLightID->try_emplace( "KANJO", 2 );
    vehLightID->try_emplace( "IMORGON", 1 );
    vehLightID->try_emplace( "OPENWHEEL1", 1 ); // mpsum
    vehLightID->try_emplace( "COQUETTE4", 0 );
    vehLightID->try_emplace( "TIGON", 0 );
    vehLightID->try_emplace( "LANDSTAL2", 0 );
    vehLightID->try_emplace( "PENUMBRA2", 1 );
    vehLightID->try_emplace( "YOSEMITE3", 2 );
    vehLightID->try_emplace( "CLUB", 1 );
    vehLightID->try_emplace( "SEMINOLE2", 1 );
    vehLightID->try_emplace( "OPENWHEEL2", 1 );
    vehLightID->try_emplace( "GAUNTLET5", 2 );
    vehLightID->try_emplace( "DUKES3", 2 );
    vehLightID->try_emplace( "GLENDALE2", 1 );
    vehLightID->try_emplace( "PEYOTE3", 2 );
    vehLightID->try_emplace( "MANANA2", 2 );
    vehLightID->try_emplace( "SQUADDIE", 0 ); // mpheist4
    vehLightID->try_emplace( "ANNIHILATOR", 1 );
    vehLightID->try_emplace( "VETO", 1 );
    vehLightID->try_emplace( "VETO2", 1 );
    vehLightID->try_emplace( "ITALIRSX", 0 );
    vehLightID->try_emplace( "TOREADOR", 0 );
    vehLightID->try_emplace( "SLAMTRUCK", 2 );
    vehLightID->try_emplace( "WEEVIL", 2 );
    vehLightID->try_emplace( "ALKONOST", 2 );
    vehLightID->try_emplace( "VETIR", 2 );
    vehLightID->try_emplace( "PATROLBOAT", 2 );
    vehLightID->try_emplace( "DINGHY5", 1 );
    vehLightID->try_emplace( "AVISA", 0 );
    vehLightID->try_emplace( "BRIOSO2", 2 );
    vehLightID->try_emplace( "VERUS", 1 );
    vehLightID->try_emplace( "LONGFIN", 0 );
    vehLightID->try_emplace( "SEASPARROW2", 1 );
    vehLightID->try_emplace( "SEASPARROW3", 1 );
    vehLightID->try_emplace( "WINKY", 0 );
    vehLightID->try_emplace( "MANCHEZ2", 0 );
    vehLightID->try_emplace( "KOSATKA", 0 );
    vehLightID->try_emplace( "FREIGHTCAR2", 0 ); // mptuner
    vehLightID->try_emplace( "EUROS", 0 );
    vehLightID->try_emplace( "DOMINATOR8", 2 );
    vehLightID->try_emplace( "SULTAN3", 2 );
    vehLightID->try_emplace( "TAILGATER2", 1 );
    vehLightID->try_emplace( "FUTO2", 2 );
    vehLightID->try_emplace( "RT3000", 2 );
    vehLightID->try_emplace( "DOMINATOR7", 0 );
    vehLightID->try_emplace( "COMET6", 0 );
    vehLightID->try_emplace( "GROWLER", 0 );
    vehLightID->try_emplace( "CALICO", 0 );
    vehLightID->try_emplace( "REMUS", 2 );
    vehLightID->try_emplace( "ZR350", 2 );
    vehLightID->try_emplace( "PREVION", 2 );
    vehLightID->try_emplace( "WARRENER2", 2 );
    vehLightID->try_emplace( "CYPHER", 0 );
    vehLightID->try_emplace( "JESTER4", 0 );
    vehLightID->try_emplace( "VECTRE", 0 );
    vehLightID->try_emplace( "SHINOBI", 1 ); // mpcontract
    vehLightID->try_emplace( "REEVER", 0 );
    vehLightID->try_emplace( "CHAMPION", 1 );
    vehLightID->try_emplace( "CINQUEMILA", 0 );
    vehLightID->try_emplace( "IWAGEN", 0 );
    vehLightID->try_emplace( "COMET7", 0 );
    vehLightID->try_emplace( "ASTRON", 0 );
    vehLightID->try_emplace( "BALLER7", 1 );
    vehLightID->try_emplace( "BUFFALO4", 1 );
    vehLightID->try_emplace( "DEITY", 1 );
    vehLightID->try_emplace( "JUBILEE", 0 );
    vehLightID->try_emplace( "GRANGER2", 1 );
    vehLightID->try_emplace( "IGNUS", 0 );
    vehLightID->try_emplace( "PATRIOT3", 0 );
    vehLightID->try_emplace( "YOUGA4", 0 );
    vehLightID->try_emplace( "ZENO", 0 );
    vehLightID->try_emplace( "MULE5", 0 );
    vehLightID->try_emplace( "S95", 0 ); // mpg9ec
    vehLightID->try_emplace( "ASTRON2", 0 );
    vehLightID->try_emplace( "CYCLONE2", 0 );
    vehLightID->try_emplace( "IGNUS2", 0 );
    vehLightID->try_emplace( "ARBITERGT", 2 );
    vehLightID->try_emplace( "OMNISEGT", 1 ); // mpsum2
    vehLightID->try_emplace( "SENTINEL4", 2 );
    vehLightID->try_emplace( "CONADA", 0 );
    vehLightID->try_emplace( "RUINER4", 1 );
    vehLightID->try_emplace( "BRIOSO3", 2 );
    vehLightID->try_emplace( "CORSITA", 1 );
    vehLightID->try_emplace( "DRAUGUR", 1 );
    vehLightID->try_emplace( "KANJOSJ", 8 );
    vehLightID->try_emplace( "POSTLUDE", 2 );
    vehLightID->try_emplace( "TORERO2", 0 );
    vehLightID->try_emplace( "VIGERO2", 1 );
    vehLightID->try_emplace( "LM87", 2 );
    vehLightID->try_emplace( "TENF", 1 );
    vehLightID->try_emplace( "TENF2", 1 );
    vehLightID->try_emplace( "RHINEHART", 1 );
    vehLightID->try_emplace( "GREENWOOD", 1 );
    vehLightID->try_emplace( "SM722", 0 );
    vehLightID->try_emplace( "WEEVIL2", 2 );
    vehLightID->try_emplace( "TAHOMA", 2 ); // mpchristmas3
    vehLightID->try_emplace( "BROADWAY", 1 );
    vehLightID->try_emplace( "R300", 1 );
    vehLightID->try_emplace( "TULIP2", 1 );
    vehLightID->try_emplace( "EVERON2", 1 );
    vehLightID->try_emplace( "VIRTUE", 1 );
    vehLightID->try_emplace( "BOOR", 1 );
    vehLightID->try_emplace( "ISSI8", 1 );
    vehLightID->try_emplace( "PANTHERE", 1 );
    vehLightID->try_emplace( "ENTITY3", 1 );
    vehLightID->try_emplace( "EUDORA", 2 );
    vehLightID->try_emplace( "POWERSURGE", 1 );
    vehLightID->try_emplace( "JOURNEY2", 0 );
    vehLightID->try_emplace( "CARGOPLANE2", 0 );
    vehLightID->try_emplace( "BRICKADE2", 0 );
    vehLightID->try_emplace( "MANCHEZ3", 0 );
    vehLightID->try_emplace( "SURFER3", 8 );
    vehLightID->try_emplace( "INDUCTOR2", 0 ); // mp2023_01
    vehLightID->try_emplace( "INDUCTOR", 0 );
    vehLightID->try_emplace( "RAIJU", 1 );
    vehLightID->try_emplace( "MONSTROCITI", 1 );
    vehLightID->try_emplace( "COUREUR", 0 );
    vehLightID->try_emplace( "RATEL", 1 );
    vehLightID->try_emplace( "STINGERTT", 1 );
    vehLightID->try_emplace( "AVENGER3", 1 );
    vehLightID->try_emplace( "AVENGER4", 1 );
    vehLightID->try_emplace( "CLIQUE2", 2 );
    vehLightID->try_emplace( "STREAMER216", 1 );
    vehLightID->try_emplace( "BRIGHAM", 2 );
    vehLightID->try_emplace( "GAUNTLET6", 1 );
    vehLightID->try_emplace( "CONADA2", 1 );
    vehLightID->try_emplace( "L35", 0 );
    vehLightID->try_emplace( "SPEEDO5", 0 );
    vehLightID->try_emplace( "BUFFALO5", 1 );
    vehLightID->try_emplace( "POLICE5", 1 ); // mp2023_02
    vehLightID->try_emplace( "VIVANITE", 0 );
    vehLightID->try_emplace( "TERMINUS", 1 );
    vehLightID->try_emplace( "IMPALER6", 2 );
    vehLightID->try_emplace( "DORADO", 0 );
    vehLightID->try_emplace( "TOWTRUCK4", 7 );
    vehLightID->try_emplace( "TOWTRUCK3", 7 );
    vehLightID->try_emplace( "BENSON2", 7 );
    vehLightID->try_emplace( "IMPALER5", 2 );
    vehLightID->try_emplace( "VIGERO3", 1 );
    vehLightID->try_emplace( "CAVALCADE3", 1 );
    vehLightID->try_emplace( "DRIFTFR36", 1 );
    vehLightID->try_emplace( "DRIFTJESTER", 0 );
    vehLightID->try_emplace( "DRIFTFUTO", 2 );
    vehLightID->try_emplace( "DRIFTEUROS", 0 );
    vehLightID->try_emplace( "DRIFTYOSEMITE", 2 );
    vehLightID->try_emplace( "DRIFTTAMPA", 2 );
    vehLightID->try_emplace( "DRIFTREMUS", 2 );
    vehLightID->try_emplace( "POLGAUNTLET", 1 );
    vehLightID->try_emplace( "ALEUTIAN", 0 );
    vehLightID->try_emplace( "FREIGHT2", 0 );
    vehLightID->try_emplace( "DOMINATOR9", 1 );
    vehLightID->try_emplace( "TURISMO3", 1 );
    vehLightID->try_emplace( "BOXVILLE6", 0 );
    vehLightID->try_emplace( "TVTRAILER2", 0 );
    vehLightID->try_emplace( "FR36", 1 );
    vehLightID->try_emplace( "ASTEROPE2", 1 );
    vehLightID->try_emplace( "BOATTRAILER2", 0 );
    vehLightID->try_emplace( "BOATTRAILER3", 0 );
    vehLightID->try_emplace( "PHANTOM4", 7 );
    vehLightID->try_emplace( "TRAILERS5", 0 );
    vehLightID->try_emplace( "BALLER8", 1 );
    vehLightID->try_emplace( "POLDOMINATOR10", 2 ); // mp2024_01
    vehLightID->try_emplace( "YOSEMITE1500", 0 );
    vehLightID->try_emplace( "POLDORADO", 0 );
    vehLightID->try_emplace( "COQUETTE5", 1 );
    vehLightID->try_emplace( "PIZZABOY", 0 );
    vehLightID->try_emplace( "PARAGON3", 1 );
    vehLightID->try_emplace( "ENVISAGE", 2 );
    vehLightID->try_emplace( "POLIMPALER6", 2 );
    vehLightID->try_emplace( "EUROSX32", 1 );
    vehLightID->try_emplace( "POLICET3", 0 );
    vehLightID->try_emplace( "PIPISTRELLO", 1 );
    vehLightID->try_emplace( "CASTIGATOR", 1 );
    vehLightID->try_emplace( "DOMINATOR10", 2 );
    vehLightID->try_emplace( "NIOBE", 0 );
    vehLightID->try_emplace( "POLIMPALER5", 2 );
    vehLightID->try_emplace( "POLGREENWOOD", 0 );
    vehLightID->try_emplace( "DRIFTNEBULA", 2 );
    vehLightID->try_emplace( "DRIFTSENTINEL", 2 );
    vehLightID->try_emplace( "DRIFTCYPHER", 0 );
    vehLightID->try_emplace( "DRIFTVORSCHLAG", 1 );
    vehLightID->try_emplace( "VORSCHLAGHAMMER", 1 );
    vehLightID->try_emplace( "FIREBOLT", 1 ); // mp2024_02
    vehLightID->try_emplace( "COQUETTE6", 1 );
    vehLightID->try_emplace( "BANSHEE3", 1 );
    vehLightID->try_emplace( "TITAN2", 1 );
    vehLightID->try_emplace( "CHAVOSV6", 1 );
    vehLightID->try_emplace( "FREIGHTCAR3", 0 );
    vehLightID->try_emplace( "CARGOBOB5", 1 );
    vehLightID->try_emplace( "DUSTER2", 1 );
    vehLightID->try_emplace( "JESTER5", 0 );
    vehLightID->try_emplace( "YOUGA5", 0 );
    vehLightID->try_emplace( "DRIFTFUTO2", 2 );
    vehLightID->try_emplace( "DRIFTJESTER3", 1 );
    vehLightID->try_emplace( "DRIFTCHEBUREK", 2 );
    vehLightID->try_emplace( "POLCOQUETTE4", 0 );
    vehLightID->try_emplace( "POLFACTION2", 2 );
    vehLightID->try_emplace( "POLTERMINUS", 1 );
    vehLightID->try_emplace( "POLCARACARA", 1 );
    vehLightID->try_emplace( "POLURANUS", 2 );

    bool restoreLightsSettings = false;

    while( true )
    {
        if( IsKeyJustUp( reloadKey ) )
        {
            loadConfigFromFile();
        }
        if( IsKeyJustUp( toggleKey ) )
        {
            restoreLightsSettings = isModEnabled;
            isModEnabled          = !isModEnabled;
        }
        if( isModEnabled )
        {
            update();
        }
        else if( restoreLightsSettings )
        {
            RestoreOriginalLightsSettings();
        }
        WAIT( 0 );
    }
}

void UnloadScript()
{
    if( vehLightID != nullptr )
    {
        delete vehLightID;
        vehLightID = nullptr;
    }
}
