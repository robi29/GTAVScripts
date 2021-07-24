#include "..\Common\inc\script.h"
#include "..\Common\inc\keyboard.h"

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
    { 0xFF, 0x00, 0x00 }  // 8 (for testing)
};

std::unordered_map<std::string, uint8_t>* vehLightID = nullptr;

__forceinline void update()
{
    const int vehicleArrayCount = worldGetAllVehicles( vehsArray, vehsArraySize );
    const int vehicleCount      = ( vehicleArrayCount < 300 )
             ? vehicleArrayCount
             : 300;

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

                const int left  = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME( vehicleID, "headlight_l" );
                const int right = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME( vehicleID, "headlight_r" );

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
    for( unsigned i = 0; i < vehsArraySize; ++i )
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

    vehLightID->emplace( "NINEF", 1 );
    vehLightID->emplace( "NINEF2", 1 );
    vehLightID->emplace( "ASEA", 0 );
    vehLightID->emplace( "BOXVILLE2", 0 );
    vehLightID->emplace( "BULLDOZE", 7 ); // BULLDOZER
    vehLightID->emplace( "CHEETAH", 1 );
    vehLightID->emplace( "COGCABRI", 1 ); // COGCABRIO
    vehLightID->emplace( "DUBSTA", 1 );
    vehLightID->emplace( "DUBSTA2", 0 );
    vehLightID->emplace( "EMPEROR", 0 );
    vehLightID->emplace( "ENTITYXF", 1 );
    vehLightID->emplace( "FIRETRUK", 0 );
    vehLightID->emplace( "FQ2", 0 );
    vehLightID->emplace( "INFERNUS", 1 );
    vehLightID->emplace( "JACKAL", 0 );
    vehLightID->emplace( "JOURNEY", 0 );
    vehLightID->emplace( "JB700", 0 );
    vehLightID->emplace( "ORACLE", 1 );
    vehLightID->emplace( "PATRIOT", 1 );
    vehLightID->emplace( "RADI", 1 );
    vehLightID->emplace( "ROMERO", 0 );
    vehLightID->emplace( "STINGER", 2 );
    vehLightID->emplace( "STOCKADE", 0 );
    vehLightID->emplace( "SUPERD", 1 );
    vehLightID->emplace( "TAILGATE", 1 ); // TAILGATER
    vehLightID->emplace( "TORNADO", 2 );
    vehLightID->emplace( "UTILTRUC", 0 ); // UTILLITRUCK
    vehLightID->emplace( "UTILLITRUCK2", 0 );
    vehLightID->emplace( "VOODOO2", 2 );
    vehLightID->emplace( "SCORCHER", 0 );
    vehLightID->emplace( "POLICEB", 1 );
    vehLightID->emplace( "HEXER", 1 );
    vehLightID->emplace( "BUZZARD", 0 );
    vehLightID->emplace( "POLMAV", 0 );
    vehLightID->emplace( "CUBAN800", 0 );
    vehLightID->emplace( "JET", 0 );
    vehLightID->emplace( "TITAN", 0 );
    vehLightID->emplace( "SQUALO", 0 );
    vehLightID->emplace( "MARQUIS", 0 );
    vehLightID->emplace( "FREIGHTCAR", 0 );
    vehLightID->emplace( "FREIGHT", 0 );
    vehLightID->emplace( "FREIGHTCONT1", 0 );
    vehLightID->emplace( "FREIGHTCONT2", 0 );
    vehLightID->emplace( "FREIGHTGRAIN", 0 );
    vehLightID->emplace( "TANKERCAR", 0 );
    vehLightID->emplace( "METROTRAIN", 0 );
    vehLightID->emplace( "TRAILERS", 0 );
    vehLightID->emplace( "TANKER", 0 );
    vehLightID->emplace( "TRAILERLOGS", 0 );
    vehLightID->emplace( "TR2", 0 );
    vehLightID->emplace( "TR3", 0 );
    vehLightID->emplace( "PICADOR", 0 );
    vehLightID->emplace( "POLICEO1", 2 ); // POLICEOLD1
    vehLightID->emplace( "POLICEO2", 2 ); // ADDED
    vehLightID->emplace( "ASTROPE", 0 );  // ASTEROPE
    vehLightID->emplace( "BANSHEE", 1 );
    vehLightID->emplace( "BUFFALO", 1 );
    vehLightID->emplace( "BULLET", 1 );
    vehLightID->emplace( "F620", 1 );
    vehLightID->emplace( "HANDLER", 7 );
    vehLightID->emplace( "RUINER", 2 );
    vehLightID->emplace( "GBURRITO", 0 );
    vehLightID->emplace( "TRACTOR2", 0 );
    vehLightID->emplace( "PENUMBRA", 0 );
    vehLightID->emplace( "SUBMERS", 3 ); // SUBMERSIBLE
    vehLightID->emplace( "DOCKTUG", 7 );
    vehLightID->emplace( "DOCKTRAILER", 0 );
    vehLightID->emplace( "SULTAN", 0 );
    vehLightID->emplace( "DILETTAN", 1 ); // DILETTANTE
    vehLightID->emplace( "FUTO", 2 );
    vehLightID->emplace( "HABANERO", 0 );
    vehLightID->emplace( "INTRUDER", 0 );
    vehLightID->emplace( "LANDSTAL", 1 ); // LANDSTALKER
    vehLightID->emplace( "MINIVAN", 0 );
    vehLightID->emplace( "SCHAFTER2", 1 );
    vehLightID->emplace( "SERRANO", 1 );
    vehLightID->emplace( "MANANA", 2 );
    vehLightID->emplace( "SEASHARK2", 0 );
    vehLightID->emplace( "YOUGA", 0 );
    vehLightID->emplace( "PREMIER", 1 );
    vehLightID->emplace( "SPEEDO", 0 );
    vehLightID->emplace( "WASHINGT", 0 ); // WASHINGTON
    vehLightID->emplace( "ANNIHILATOR", 0 );
    vehLightID->emplace( "BLAZER2", 0 );
    vehLightID->emplace( "CRUISER", 0 );
    vehLightID->emplace( "RAKETRAILER", 0 );
    vehLightID->emplace( "CARGOPLANE", 0 );
    vehLightID->emplace( "DUMP", 7 );
    vehLightID->emplace( "PONY", 2 );
    vehLightID->emplace( "LGUARD", 0 );
    vehLightID->emplace( "SENTINEL", 1 );
    vehLightID->emplace( "SENTINEL2", 1 );
    vehLightID->emplace( "COMET2", 1 );
    vehLightID->emplace( "STINGERG", 2 ); // STINGERGT
    vehLightID->emplace( "INGOT", 2 );
    vehLightID->emplace( "PEYOTE", 2 );
    vehLightID->emplace( "STANIER", 0 );
    vehLightID->emplace( "STRATUM", 2 );
    vehLightID->emplace( "AKUMA", 1 );
    vehLightID->emplace( "CARBON", 1 ); // ADDED
    vehLightID->emplace( "BATI", 1 );
    vehLightID->emplace( "BATI2", 1 );
    vehLightID->emplace( "PCJ", 0 );
    vehLightID->emplace( "DLOADER", 2 );
    vehLightID->emplace( "PRAIRIE", 1 );
    vehLightID->emplace( "DUSTER", 0 );
    vehLightID->emplace( "ISSI2", 1 );
    vehLightID->emplace( "TRAILERS2", 0 );
    vehLightID->emplace( "TVTRAILER", 0 );
    vehLightID->emplace( "CUTTER", 7 );
    vehLightID->emplace( "TRFLAT", 0 );
    vehLightID->emplace( "TORNADO2", 2 );
    vehLightID->emplace( "TORNADO3", 2 );
    vehLightID->emplace( "TRIBIKE", 0 );
    vehLightID->emplace( "TRIBIKE2", 0 );
    vehLightID->emplace( "TRIBIKE3", 0 );
    vehLightID->emplace( "PROPTRAILER", 0 );
    vehLightID->emplace( "BURRITO2", 0 );
    vehLightID->emplace( "DUNE", 0 );
    vehLightID->emplace( "FELTZER2", 1 );
    vehLightID->emplace( "BLISTA", 1 );
    vehLightID->emplace( "BAGGER", 0 );
    vehLightID->emplace( "VOLTIC", 1 );
    vehLightID->emplace( "FUGITIVE", 1 );
    vehLightID->emplace( "FELON", 1 );
    vehLightID->emplace( "PBUS", 0 );
    vehLightID->emplace( "ARMYTRAILER", 0 );
    vehLightID->emplace( "POLICET", 0 );
    vehLightID->emplace( "SPEEDO2", 2 );
    vehLightID->emplace( "FELON2", 1 );
    vehLightID->emplace( "BMX", 0 );
    vehLightID->emplace( "EXEMPLAR", 1 );
    vehLightID->emplace( "FUSILADE", 0 );
    vehLightID->emplace( "BOATTRAILER", 0 );
    vehLightID->emplace( "CAVCADE", 1 ); // CAVALCADE
    vehLightID->emplace( "SURGE", 0 );
    vehLightID->emplace( "BUCCANEE", 2 ); // BUCCANEER
    vehLightID->emplace( "NEMESIS", 1 );
    vehLightID->emplace( "ARMYTANKER", 0 );
    vehLightID->emplace( "ROCOTO", 1 );
    vehLightID->emplace( "STOCKADE3", 0 );
    vehLightID->emplace( "REBEL02", 2 );  // REBEL2
    vehLightID->emplace( "SCHWARZE", 1 ); // SCHWARZER
    vehLightID->emplace( "SCRAP", 7 );
    vehLightID->emplace( "SANDKING", 0 );
    vehLightID->emplace( "SANDKIN2", 0 ); // SANDKING2
    vehLightID->emplace( "CARBONIZ", 1 ); // CARBONIZZARE
    vehLightID->emplace( "RUMPO", 0 );
    vehLightID->emplace( "PRIMO", 0 );
    vehLightID->emplace( "SABREGT", 2 );
    vehLightID->emplace( "REGINA", 2 );
    vehLightID->emplace( "JETMAX", 0 );
    vehLightID->emplace( "TROPIC", 0 );
    vehLightID->emplace( "VIGERO", 0 );
    vehLightID->emplace( "POLICE2", 1 );
    vehLightID->emplace( "STRETCH", 1 );
    vehLightID->emplace( "DINGHY2", 0 );
    vehLightID->emplace( "BOXVILLE", 0 );
    vehLightID->emplace( "LUXOR", 0 );
    vehLightID->emplace( "POLICD3", 1 );
    vehLightID->emplace( "TRAILERS3", 0 );
    vehLightID->emplace( "DOUBLE", 1 );
    vehLightID->emplace( "TRACTOR", 7 );
    vehLightID->emplace( "BIFF", 7 );
    vehLightID->emplace( "DOMINATO", 0 ); // DOMINATOR
    vehLightID->emplace( "HAULER", 7 );
    vehLightID->emplace( "PACKER", 7 );
    vehLightID->emplace( "PHOENIX", 2 );
    vehLightID->emplace( "SADLER", 0 );
    vehLightID->emplace( "SADLER2", 0 );
    vehLightID->emplace( "DAEMON", 0 );
    vehLightID->emplace( "COACH", 0 );
    vehLightID->emplace( "TORNADO4", 2 );
    vehLightID->emplace( "RATLOADER", 2 );
    vehLightID->emplace( "RAPIDGT", 1 );
    vehLightID->emplace( "RAPIDGT2", 1 );
    vehLightID->emplace( "SURANO", 1 );
    vehLightID->emplace( "BFINJECT", 2 ); // BFINJECTION
    vehLightID->emplace( "BISON2", 0 );
    vehLightID->emplace( "BISON3", 0 );
    vehLightID->emplace( "BODHI2", 0 );
    vehLightID->emplace( "BURRITO", 0 );
    vehLightID->emplace( "BURRITO4", 0 );
    vehLightID->emplace( "RUBBLE", 7 );
    vehLightID->emplace( "TIPTRUCK", 7 );
    vehLightID->emplace( "TIPTRUCK2", 7 );
    vehLightID->emplace( "MIXER", 7 );
    vehLightID->emplace( "MIXER2", 7 );
    vehLightID->emplace( "PHANTOM", 7 );
    vehLightID->emplace( "POUNDER", 7 );
    vehLightID->emplace( "BUZZARD2", 0 );
    vehLightID->emplace( "FROGGER", 0 );
    vehLightID->emplace( "AIRTUG", 7 );
    vehLightID->emplace( "BENSON", 7 );
    vehLightID->emplace( "RIPLEY", 7 );
    vehLightID->emplace( "AMBULAN", 0 ); // AMBULANCE
    vehLightID->emplace( "FORK", 7 );    // FORKLIFT
    vehLightID->emplace( "GRANGER", 0 );
    vehLightID->emplace( "PRANGER", 0 );
    vehLightID->emplace( "TRAILERSMALL", 0 );
    vehLightID->emplace( "BARRACKS", 0 );
    vehLightID->emplace( "BARRACKS2", 0 );
    vehLightID->emplace( "CRUSADER", 0 );
    vehLightID->emplace( "UTILLITRUCK3", 0 );
    vehLightID->emplace( "SHERIFF", 0 );
    vehLightID->emplace( "MONROE", 2 );
    vehLightID->emplace( "MULE", 0 );
    vehLightID->emplace( "TACO", 0 );
    vehLightID->emplace( "TRASH", 7 );
    vehLightID->emplace( "DINGHY", 1 );
    vehLightID->emplace( "BLAZER", 0 );
    vehLightID->emplace( "MAVERICK", 0 );
    vehLightID->emplace( "CARGOBOB", 0 );
    vehLightID->emplace( "CARGOBOB3", 0 );
    vehLightID->emplace( "STUNT", 0 );
    vehLightID->emplace( "EMPEROR3", 2 );
    vehLightID->emplace( "CADDY", 0 );
    vehLightID->emplace( "EMPEROR2", 2 );
    vehLightID->emplace( "SURFER2", 2 );
    vehLightID->emplace( "TOWTRUCK", 7 );
    vehLightID->emplace( "TWOTRUCK2", 7 );
    vehLightID->emplace( "BALLER", 1 );
    vehLightID->emplace( "SURFER", 2 );
    vehLightID->emplace( "MAMMATUS", 0 );
    vehLightID->emplace( "RIOT", 0 );
    vehLightID->emplace( "VELUM", 0 );
    vehLightID->emplace( "RANCHERX12", 0 );
    vehLightID->emplace( "CADDY2", 0 );
    vehLightID->emplace( "AIRBUS", 0 );
    vehLightID->emplace( "RENTBUS", 0 ); // RENTALBUS
    vehLightID->emplace( "GRESLEY", 1 );
    vehLightID->emplace( "ZION", 0 );
    vehLightID->emplace( "ZION2", 0 );
    vehLightID->emplace( "RUFFIAN", 1 );
    vehLightID->emplace( "ADDER", 1 );
    vehLightID->emplace( "VACCA", 1 );
    vehLightID->emplace( "BOXVILLE3", 0 );
    vehLightID->emplace( "SUNTRAP", 0 );
    vehLightID->emplace( "BOBCATXL", 2 );
    vehLightID->emplace( "BURRITO3", 0 );
    vehLightID->emplace( "POLICE4", 1 );
    vehLightID->emplace( "CABLECAR", 0 );
    vehLightID->emplace( "BLIMP", 0 );
    vehLightID->emplace( "BUS", 0 );
    vehLightID->emplace( "DILETTANTE2", 1 );
    vehLightID->emplace( "REBEL01", 0 ); // REBEL
    vehLightID->emplace( "SKYLIFT", 0 );
    vehLightID->emplace( "SHAMAL", 0 );
    vehLightID->emplace( "GRAINTRAILER", 0 );
    vehLightID->emplace( "VADER", 1 );
    vehLightID->emplace( "SHERIFF2", 0 );
    vehLightID->emplace( "BALETRAILER", 0 );
    vehLightID->emplace( "TOURBUS", 0 );
    vehLightID->emplace( "FIXTER", 0 );
    vehLightID->emplace( "ORACLE2", 1 );
    vehLightID->emplace( "BALLER2", 0 );
    vehLightID->emplace( "BUFFALO02", 0 ); // BUFFALO2
    vehLightID->emplace( "CAVALCADE2", 1 );
    vehLightID->emplace( "COQUETTE", 1 );
    vehLightID->emplace( "TRACTOR3", 2 );
    vehLightID->emplace( "GAUNTLET", 1 );
    vehLightID->emplace( "MESA", 0 );
    vehLightID->emplace( "POLICE", 1 );
    vehLightID->emplace( "CARGOBOB2", 0 );
    vehLightID->emplace( "TAXI", 0 );
    vehLightID->emplace( "SANCHEZ", 0 );
    vehLightID->emplace( "FLATBED", 0 );
    vehLightID->emplace( "SEMINOLE", 0 );
    vehLightID->emplace( "MOWER", 0 );
    vehLightID->emplace( "ZTYPE", 0 );
    vehLightID->emplace( "PREDATOR", 1 );
    vehLightID->emplace( "RUMPO2", 0 );
    vehLightID->emplace( "PONY2", 0 );
    vehLightID->emplace( "BJXL", 1 );
    vehLightID->emplace( "CAMPER", 0 );
    vehLightID->emplace( "RANCHERX", 0 ); // RANCHERXL
    vehLightID->emplace( "FAGGIO2", 0 );
    vehLightID->emplace( "LAZER", 0 );
    vehLightID->emplace( "SEASHARK", 0 );
    vehLightID->emplace( "BISON", 1 );
    vehLightID->emplace( "FBI", 1 );
    vehLightID->emplace( "FBI2", 0 );
    vehLightID->emplace( "MULE2", 0 );
    vehLightID->emplace( "RHINO", 0 );
    vehLightID->emplace( "BURRITO5", 0 );
    vehLightID->emplace( "ASES2", 2 );
    vehLightID->emplace( "MESA2", 0 );
    vehLightID->emplace( "MESA3", 0 );
    vehLightID->emplace( "FROGGER2", 0 );
    vehLightID->emplace( "HOTKNIFE", 1 );
    vehLightID->emplace( "ELEGY2", 1 );
    vehLightID->emplace( "KHAMEL", 1 ); // KHAMELION
    vehLightID->emplace( "DUNE2", 6 );
    vehLightID->emplace( "ARMYTRAILER2", 0 );
    vehLightID->emplace( "FREIGHTTRAILER", 0 );
    vehLightID->emplace( "TR4", 0 );
    vehLightID->emplace( "BLAZER03", 0 );  // BLAZER3
    vehLightID->emplace( "SANCHEZ01", 0 ); // ADDED
    vehLightID->emplace( "SANCHEZ02", 0 ); // SANCHEZ2
    vehLightID->emplace( "VERLIER", 1 );   // mpapartment // VERLIERER2
    vehLightID->emplace( "MAMBA", 2 );
    vehLightID->emplace( "NITESHAD", 2 ); // NIGHTSHADE
    vehLightID->emplace( "COG55", 1 );
    vehLightID->emplace( "COG552", 1 );
    vehLightID->emplace( "COGNOSC", 1 );  // COGNOSCENTI
    vehLightID->emplace( "COGNOSC2", 1 ); // COGNOSCENTI2
    vehLightID->emplace( "SCHAFTER3", 1 );
    vehLightID->emplace( "SCHAFTER4", 1 );
    vehLightID->emplace( "SCHAFTER5", 1 );
    vehLightID->emplace( "SCHAFTER6", 1 );
    vehLightID->emplace( "LIMO2", 1 );
    vehLightID->emplace( "BALLER3", 0 );
    vehLightID->emplace( "BALLER4", 0 );
    vehLightID->emplace( "BALLER5", 0 );
    vehLightID->emplace( "BALLER6", 0 );
    vehLightID->emplace( "SEASHARK3", 0 );
    vehLightID->emplace( "DINGHY4", 0 );
    vehLightID->emplace( "TROPIC2", 0 );
    vehLightID->emplace( "SPEEDER2", 0 );
    vehLightID->emplace( "TORO2", 0 );
    vehLightID->emplace( "CARGOBOB4", 0 );
    vehLightID->emplace( "SUPERVOLITO", 0 );
    vehLightID->emplace( "SUPERVOLITO2", 0 );
    vehLightID->emplace( "VALKYRIE2", 0 );
    vehLightID->emplace( "BIFTA", 0 ); // mpbeach
    vehLightID->emplace( "SPEEDER", 0 );
    vehLightID->emplace( "KALAHARI", 0 );
    vehLightID->emplace( "PARADISE", 0 );
    vehLightID->emplace( "TORNADO6", 2 ); //mpbiker
    vehLightID->emplace( "RAPTOR", 1 );
    vehLightID->emplace( "VORTEX", 1 );
    vehLightID->emplace( "AVARUS", 1 );
    vehLightID->emplace( "SANCTUS", 4 );
    vehLightID->emplace( "FAGGIO3", 0 );
    vehLightID->emplace( "FAGGIO", 0 );
    vehLightID->emplace( "FAGGION", 0 ); // ADDED
    vehLightID->emplace( "HAKUCHOU2", 1 );
    vehLightID->emplace( "NIGHTBLADE", 0 );
    vehLightID->emplace( "YOUGA2", 1 );
    vehLightID->emplace( "CHIMERA", 0 );
    vehLightID->emplace( "ESSKEY", 0 );
    vehLightID->emplace( "ZOMBIEA", 0 );
    vehLightID->emplace( "WOLFSBANE", 0 );
    vehLightID->emplace( "DAEMON2", 0 );
    vehLightID->emplace( "SHOTARO", 0 );
    vehLightID->emplace( "RATBIKE", 0 );
    vehLightID->emplace( "ZOMBIEB", 0 );
    vehLightID->emplace( "DEFILER", 0 );
    vehLightID->emplace( "MANCHEZ", 0 );
    vehLightID->emplace( "BLAZER4", 0 );
    vehLightID->emplace( "JESTER", 0 ); // mpbusiness
    vehLightID->emplace( "TURISMOR", 0 );
    vehLightID->emplace( "ALPHA", 1 );
    vehLightID->emplace( "VESTRA", 0 );
    vehLightID->emplace( "ZENTORNO", 0 ); // mpbusiness2
    vehLightID->emplace( "MASSACRO", 1 );
    vehLightID->emplace( "HUNTLEY", 1 );
    vehLightID->emplace( "THRUST", 1 );
    vehLightID->emplace( "SLAMVAN", 2 );  // mpchristmas2
    vehLightID->emplace( "RLOADER2", 2 ); // RATLOADER2
    vehLightID->emplace( "JESTER2", 0 );
    vehLightID->emplace( "MASSACRO2", 1 );
    vehLightID->emplace( "NIMBUS", 0 ); // mpexecutive
    vehLightID->emplace( "XLS", 1 );
    vehLightID->emplace( "XLS2", 1 );
    vehLightID->emplace( "SEVEN70", 1 );
    vehLightID->emplace( "REAPER", 1 );
    vehLightID->emplace( "FMJ", 1 );
    vehLightID->emplace( "PFISTER811", 1 );
    vehLightID->emplace( "BESTIAGTS", 1 );
    vehLightID->emplace( "BRICKADE", 0 );
    vehLightID->emplace( "RUMPO3", 0 );
    vehLightID->emplace( "PROTOTIPO", 1 );
    vehLightID->emplace( "WINDSOR2", 1 );
    vehLightID->emplace( "VOLATUS", 0 );
    vehLightID->emplace( "TUG", 0 );
    vehLightID->emplace( "TRAILERS4", 0 ); // mpgunrunning
    vehLightID->emplace( "XA21", 0 );
    vehLightID->emplace( "VAGNER", 1 );
    vehLightID->emplace( "CADDY3", 0 );
    vehLightID->emplace( "PHANTOM3", 1 );
    vehLightID->emplace( "NIGHTSHARK", 1 );
    vehLightID->emplace( "CHEETAH2", 1 );
    vehLightID->emplace( "TORERO", 1 );
    vehLightID->emplace( "HAULER2", 7 );
    vehLightID->emplace( "TRAILERLARGE", 1 );
    vehLightID->emplace( "TECHNICAL3", 0 );
    vehLightID->emplace( "TAMPA3", 2 );
    vehLightID->emplace( "INSURGENT3", 1 );
    vehLightID->emplace( "APC", 2 );
    vehLightID->emplace( "HALFTRACK", 2 );
    vehLightID->emplace( "DUNE3", 0 );
    vehLightID->emplace( "TRAILERSMALL2", 0 );
    vehLightID->emplace( "ARDENT", 1 );
    vehLightID->emplace( "OPPRESSOR", 0 );
    vehLightID->emplace( "LURCHER", 2 ); // mphalloween
    vehLightID->emplace( "BTYPE2", 2 );
    vehLightID->emplace( "MULE3", 0 ); // mpheist
    vehLightID->emplace( "TRASH2", 7 );
    vehLightID->emplace( "VELUM2", 0 );
    vehLightID->emplace( "TANKER2", 0 );
    vehLightID->emplace( "ENDURO", 1 );
    vehLightID->emplace( "SAVAGE", 1 );
    vehLightID->emplace( "CASCO", 0 );
    vehLightID->emplace( "TECHNICAL", 0 );
    vehLightID->emplace( "INSURGENT", 0 );
    vehLightID->emplace( "INSURGENT2", 0 );
    vehLightID->emplace( "HYDRA", 0 );
    vehLightID->emplace( "BOXVILLE4", 0 );
    vehLightID->emplace( "GBURRITO2", 0 );
    vehLightID->emplace( "GUARDIAN", 1 );
    vehLightID->emplace( "DINGHY3", 0 );
    vehLightID->emplace( "LECTRO", 1 );
    vehLightID->emplace( "KURUMA", 1 );
    vehLightID->emplace( "KURUMA2", 1 );
    vehLightID->emplace( "BARRCAKS3", 0 );
    vehLightID->emplace( "VALKYRIE", 0 );
    vehLightID->emplace( "SLAMVAN2", 0 );
    vehLightID->emplace( "RHAPSODY", 0 ); // mphipster
    vehLightID->emplace( "WARRENER", 2 );
    vehLightID->emplace( "BLADE", 2 );
    vehLightID->emplace( "GLENDALE", 5 );
    vehLightID->emplace( "PANTO", 0 );
    vehLightID->emplace( "DUBSTA3", 1 );
    vehLightID->emplace( "PIGALLE", 5 );
    vehLightID->emplace( "NERO", 0 ); // mpimportexport
    vehLightID->emplace( "NERO2", 0 );
    vehLightID->emplace( "ELEGY", 2 );
    vehLightID->emplace( "ITALIGTB", 0 );
    vehLightID->emplace( "ITALIGTB2", 0 );
    vehLightID->emplace( "TEMPESTA", 0 );
    vehLightID->emplace( "DIABLOUS", 1 );
    vehLightID->emplace( "SPECTER", 1 );
    vehLightID->emplace( "SPECTER2", 1 );
    vehLightID->emplace( "DIABLOUS2", 1 );
    vehLightID->emplace( "COMET3", 1 );
    vehLightID->emplace( "BLAZER5", 0 );
    vehLightID->emplace( "DUNE4", 1 );
    vehLightID->emplace( "DUNE5", 1 );
    vehLightID->emplace( "RUINER2", 2 );
    vehLightID->emplace( "VOLTIC2", 1 );
    vehLightID->emplace( "PHANTOM2", 1 );
    vehLightID->emplace( "BOXVILLE5", 1 );
    vehLightID->emplace( "WASTLNDR", 0 ); // WESTERLANDER
    vehLightID->emplace( "TECHNICAL2", 1 );
    vehLightID->emplace( "PENETRATOR", 1 );
    vehLightID->emplace( "FCR", 1 );
    vehLightID->emplace( "FCR2", 1 );
    vehLightID->emplace( "RUINER3", 1 );
    vehLightID->emplace( "MONSTER", 1 ); // mpindependence
    vehLightID->emplace( "SOVEREIGN", 1 );
    vehLightID->emplace( "SULTANRS", 1 ); // mpjanuary2016
    vehLightID->emplace( "BANSHEE2", 1 );
    vehLightID->emplace( "BUCCANEE2", 2 ); // mplowrider // BUCCANEER2
    vehLightID->emplace( "VOODOO", 2 );
    vehLightID->emplace( "FACTION", 2 );
    vehLightID->emplace( "FACTION2", 2 );
    vehLightID->emplace( "MOONBEAM", 2 );
    vehLightID->emplace( "MOONBEAM2", 2 );
    vehLightID->emplace( "PRIMO2", 2 );
    vehLightID->emplace( "CHINO2", 1 );
    vehLightID->emplace( "FACTION3", 2 ); // mplowrider2
    vehLightID->emplace( "MINIVAN2", 2 );
    vehLightID->emplace( "SABREGT2", 2 );
    vehLightID->emplace( "SLAMVAN3", 2 );
    vehLightID->emplace( "TORNADO5", 2 );
    vehLightID->emplace( "VIRGO2", 2 );
    vehLightID->emplace( "VIRGO3", 2 );
    vehLightID->emplace( "INNOVATION", 1 ); // mplts
    vehLightID->emplace( "HAKUCHOU", 1 );
    vehLightID->emplace( "FURORE", 0 ); // FUROREGT
    vehLightID->emplace( "SWIFT2", 0 ); // mpluxe
    vehLightID->emplace( "LUXOR2", 0 );
    vehLightID->emplace( "FELTZER3", 2 );
    vehLightID->emplace( "OSIRIS", 1 );
    vehLightID->emplace( "VIRGO", 2 );
    vehLightID->emplace( "WINDSOR", 1 );
    vehLightID->emplace( "COQUETTE3", 2 ); // mpluxe2
    vehLightID->emplace( "VINDICATOR", 0 );
    vehLightID->emplace( "T20", 0 );
    vehLightID->emplace( "BRAWLER", 1 );
    vehLightID->emplace( "TORO", 1 );
    vehLightID->emplace( "CHINO", 1 );
    vehLightID->emplace( "MILJET", 0 ); // mppilot
    vehLightID->emplace( "BESRA", 1 );
    vehLightID->emplace( "COQUETTE2", 2 );
    vehLightID->emplace( "SWIFT", 0 );
    vehLightID->emplace( "VIGILANTE", 1 ); // mpsmuggler
    vehLightID->emplace( "BOMBUSHKA", 1 );
    vehLightID->emplace( "HOWARD", 1 );
    vehLightID->emplace( "ALPHAZ1", 1 );
    vehLightID->emplace( "SEABREEZE", 1 );
    vehLightID->emplace( "NOKOTA", 1 );
    vehLightID->emplace( "MOLOTOK", 1 );
    vehLightID->emplace( "STARLING", 1 );
    vehLightID->emplace( "HAVOK", 1 );
    vehLightID->emplace( "TULA", 1 );
    vehLightID->emplace( "MICROLIGHT", 1 );
    vehLightID->emplace( "HUNTER", 1 );
    vehLightID->emplace( "ROGUE", 1 );
    vehLightID->emplace( "PYRO", 1 );
    vehLightID->emplace( "RAPIDGT3", 2 );
    vehLightID->emplace( "MOGUL", 1 );
    vehLightID->emplace( "RETINUE", 2 );
    vehLightID->emplace( "CYCLONE", 1 );
    vehLightID->emplace( "VISIONE", 1 );
    vehLightID->emplace( "TURISMO2", 1 ); // mpspecialraces
    vehLightID->emplace( "INFERNUS2", 1 );
    vehLightID->emplace( "GP1", 1 );
    vehLightID->emplace( "RUSTON", 1 );
    vehLightID->emplace( "GARGOYLE", 0 ); // mpstunt
    vehLightID->emplace( "OMNIS", 0 );
    vehLightID->emplace( "SHEAVA", 0 );
    vehLightID->emplace( "TYRUS", 0 );
    vehLightID->emplace( "LE7B", 0 );
    vehLightID->emplace( "LYNX", 0 );
    vehLightID->emplace( "TROPOS", 2 );
    vehLightID->emplace( "TAMPA2", 2 );
    vehLightID->emplace( "BRIOSO", 0 );
    vehLightID->emplace( "BF400", 1 );
    vehLightID->emplace( "CLIFFHANGER", 1 );
    vehLightID->emplace( "CONTENDER", 0 );
    vehLightID->emplace( "E109", 0 );    // ADDED
    vehLightID->emplace( "TROPHY", 1 );  // TROPHYTRUCK
    vehLightID->emplace( "TROPHY2", 1 ); // TROPHYTRUCK2
    vehLightID->emplace( "RALLYTRUCK", 0 );
    vehLightID->emplace( "ROOSEVELT", 0 );  // mpvalentines // BTYPE
    vehLightID->emplace( "ROOSEVELT2", 2 ); // mpvalentines2 // BTYPE3
    vehLightID->emplace( "TAMPA", 2 );      // mpxmas_604490
    vehLightID->emplace( "SUBMERS2", 3 );   // spupgrade // SUBMERSIBLE2
    vehLightID->emplace( "MARSHALL", 1 );
    vehLightID->emplace( "BLIMP2", 0 );
    vehLightID->emplace( "DUKES", 2 );
    vehLightID->emplace( "DUKES2", 2 );
    vehLightID->emplace( "BUFFALO3", 0 );
    vehLightID->emplace( "DOMINATO2", 0 ); // DOMINATOR2
    vehLightID->emplace( "GAUNTLET2", 1 );
    vehLightID->emplace( "STALION", 2 );
    vehLightID->emplace( "STALION2", 2 );
    vehLightID->emplace( "BLISTA2", 2 );
    vehLightID->emplace( "BLISTA3", 2 );
    vehLightID->emplace( "ADMIRAL", 0 ); // IV Pack
    vehLightID->emplace( "ANGEL", 0 );
    vehLightID->emplace( "APC2", 0 );
    vehLightID->emplace( "BLADE2", 1 );
    vehLightID->emplace( "BOBCAT", 2 );
    vehLightID->emplace( "BODHI", 0 );
    vehLightID->emplace( "BOXVILLE6", 0 );
    vehLightID->emplace( "BRICKADE2", 0 );
    vehLightID->emplace( "BUCCANEER3", 0 );
    vehLightID->emplace( "BUS2", 0 );
    vehLightID->emplace( "CABBY", 0 );
    vehLightID->emplace( "CHAVOS", 0 );
    vehLightID->emplace( "CHAVOS2", 0 );
    vehLightID->emplace( "CHEETAH3", 1 );
    vehLightID->emplace( "CONTENDER2", 0 );
    vehLightID->emplace( "COQUETTE5", 0 );
    vehLightID->emplace( "DF8", 1 );
    vehLightID->emplace( "DIABOLUS", 0 );
    vehLightID->emplace( "DOUBLE2", 1 );
    vehLightID->emplace( "ESPERANTO", 0 );
    vehLightID->emplace( "EMPEROR4", 0 );
    vehLightID->emplace( "FELTZER", 1 );
    vehLightID->emplace( "FEROCI", 0 );
    vehLightID->emplace( "FEROCI2", 0 );
    vehLightID->emplace( "FLATBED2", 0 );
    vehLightID->emplace( "FLOATER", 1 );
    vehLightID->emplace( "FORTUNE", 0 );
    vehLightID->emplace( "FREEWAY", 0 );
    vehLightID->emplace( "FUTO3", 2 );
    vehLightID->emplace( "FXT", 1 );
    vehLightID->emplace( "GHAWAR", 0 );
    vehLightID->emplace( "HAKUMAI", 0 );
    vehLightID->emplace( "HAKUCHOU3", 0 );
    vehLightID->emplace( "HELLFURY", 0 );
    vehLightID->emplace( "HUNTLEY2", 0 );
    vehLightID->emplace( "INTERC", 1 ); // INTERCEPTOR
    vehLightID->emplace( "JB7003", 2 );
    vehLightID->emplace( "LOKUS", 0 );
    vehLightID->emplace( "LYCAN", 0 );
    vehLightID->emplace( "LYCAN2", 0 );
    vehLightID->emplace( "MARBELLE", 2 );
    vehLightID->emplace( "MERIT", 0 );
    vehLightID->emplace( "MRTASTY", 0 );
    vehLightID->emplace( "NIGHTBLADE2", 0 );
    vehLightID->emplace( "NOOSE", 0 );
    vehLightID->emplace( "NRG900", 0 );
    vehLightID->emplace( "NSTOCKADE", 0 );
    vehLightID->emplace( "PACKER2", 0 );
    vehLightID->emplace( "PERENNIAL", 0 );
    vehLightID->emplace( "PERENNIAL2", 0 );
    vehLightID->emplace( "PHOENIX2", 2 );
    vehLightID->emplace( "PINNACLE", 0 );
    vehLightID->emplace( "PMP600", 0 );
    vehLightID->emplace( "POLICE6", 0 );
    vehLightID->emplace( "POLICE7", 1 );
    vehLightID->emplace( "POLICE8", 0 );
    vehLightID->emplace( "POLPATRIOT", 1 );
    vehLightID->emplace( "PREMIER2", 0 );
    vehLightID->emplace( "PRES", 1 );
    vehLightID->emplace( "PRES2", 1 );
    vehLightID->emplace( "PSTOCKADE", 0 );
    vehLightID->emplace( "RANCHER", 0 );
    vehLightID->emplace( "REBLA2", 3 );
    vehLightID->emplace( "REEFER", 3 );
    vehLightID->emplace( "REGINA2", 0 );
    vehLightID->emplace( "REGINA3", 2 );
    vehLightID->emplace( "REVENANT", 0 );
    vehLightID->emplace( "ROM", 0 );
    vehLightID->emplace( "SABRE", 2 );
    vehLightID->emplace( "SABRE2", 2 );
    vehLightID->emplace( "SCHAFTER", 1 );
    vehLightID->emplace( "SCHAFTERGTR", 1 );
    vehLightID->emplace( "SENTINEL4", 0 );
    vehLightID->emplace( "SMUGGLER", 1 );
    vehLightID->emplace( "SOLAIR", 0 );
    vehLightID->emplace( "SOVEREIGN2", 0 );
    vehLightID->emplace( "STANIER2", 0 );
    vehLightID->emplace( "STEED", 0 );
    vehLightID->emplace( "STRATUM2", 2 );
    vehLightID->emplace( "STRETCH2", 0 );
    vehLightID->emplace( "STRETCH3", 1 );
    vehLightID->emplace( "SULTAN4", 0 );
    vehLightID->emplace( "SUPERD2", 1 );
    vehLightID->emplace( "SUPERGT", 0 );
    vehLightID->emplace( "TAXI2", 2 );
    vehLightID->emplace( "TAXI3", 0 );
    vehLightID->emplace( "TOURMAV", 0 );
    vehLightID->emplace( "TURISMO", 0 );
    vehLightID->emplace( "TYPHOON", 0 );
    vehLightID->emplace( "URANUS", 0 );
    vehLightID->emplace( "VIGERO2", 2 );
    vehLightID->emplace( "VINCENT", 0 );
    vehLightID->emplace( "VIOLATOR", 1 );
    vehLightID->emplace( "VOODOO3", 2 );
    vehLightID->emplace( "WAYFARER", 0 );
    vehLightID->emplace( "WILLARD", 0 );
    vehLightID->emplace( "WOLFSBANE2", 0 );
    vehLightID->emplace( "YANKEE", 7 );
    vehLightID->emplace( "TANKEE2", 7 );
    vehLightID->emplace( "COMET5", 1 ); // mpchristmas2017
    vehLightID->emplace( "RAIDEN", 1 );
    vehLightID->emplace( "VISERIS", 0 );
    vehLightID->emplace( "RIATA", 1 );
    vehLightID->emplace( "KAMACHO", 1 );
    vehLightID->emplace( "SC1", 1 );
    vehLightID->emplace( "AUTARCH", 1 );
    vehLightID->emplace( "SAVESTRA", 1 );
    vehLightID->emplace( "GT500", 0 );
    vehLightID->emplace( "NEON", 1 );
    vehLightID->emplace( "YOSEMITE", 2 );
    vehLightID->emplace( "HERMES", 2 );
    vehLightID->emplace( "HUSTLER", 2 );
    vehLightID->emplace( "SENTINEL3", 2 );
    vehLightID->emplace( "Z190", 1 );
    vehLightID->emplace( "KHANJALI", 1 );
    vehLightID->emplace( "BARRAGE", 1 );
    vehLightID->emplace( "VOLATOL", 2 );
    vehLightID->emplace( "AKULA", 1 );
    vehLightID->emplace( "AVENGER", 1 );
    vehLightID->emplace( "AVENGER2", 1 );
    vehLightID->emplace( "DELUXO", 1 );
    vehLightID->emplace( "STROMBERG", 1 );
    vehLightID->emplace( "CHERNOBOG", 2 );
    vehLightID->emplace( "RIOT2", 1 );
    vehLightID->emplace( "THRUSTER", 1 );
    vehLightID->emplace( "STREITER", 1 );
    vehLightID->emplace( "REVOLTER", 1 );
    vehLightID->emplace( "PARIAH", 0 );
    vehLightID->emplace( "CARACARA", 1 ); // mpassault
    vehLightID->emplace( "SEASPARROW", 1 );
    vehLightID->emplace( "ENTITY2", 1 );
    vehLightID->emplace( "JESTER3", 1 );
    vehLightID->emplace( "TYRANT", 1 );
    vehLightID->emplace( "DOMINATOR3", 1 );
    vehLightID->emplace( "HOTRING", 1 );
    vehLightID->emplace( "FLASHGT", 1 );
    vehLightID->emplace( "TEZERACT", 1 );
    vehLightID->emplace( "ELLIE", 1 );
    vehLightID->emplace( "MICHELLI", 1 );
    vehLightID->emplace( "GB200", 1 );
    vehLightID->emplace( "ISSI3", 1 );
    vehLightID->emplace( "TAIPAN", 0 );
    vehLightID->emplace( "FAGALOA", 2 );
    vehLightID->emplace( "CHEBUREK", 2 );
    vehLightID->emplace( "STAFFORD", 2 ); // mpbattle
    vehLightID->emplace( "SCRAMJET", 1 );
    vehLightID->emplace( "STRIKEFORCE", 0 );
    vehLightID->emplace( "TERBYTE", 1 );
    vehLightID->emplace( "PBUS2", 0 );
    vehLightID->emplace( "POUNDER2", 7 );
    vehLightID->emplace( "FREECRAWLER", 1 );
    vehLightID->emplace( "MULE4", 1 );
    vehLightID->emplace( "BLIMP3", 0 );
    vehLightID->emplace( "MENACER", 1 );
    vehLightID->emplace( "SWINGER", 0 );
    vehLightID->emplace( "PATRIOT2", 1 );
    vehLightID->emplace( "OPPRESSOR2", 0 );
    vehLightID->emplace( "IMPALER3", 2 ); // mpchristmas2018
    vehLightID->emplace( "MONSTER5", 1 );
    vehLightID->emplace( "SLAMVAN6", 1 );
    vehLightID->emplace( "ISSI6", 1 );
    vehLightID->emplace( "CERBERUS3", 1 );
    vehLightID->emplace( "DEATHBIKE2", 0 );
    vehLightID->emplace( "DOMINATOR6", 1 );
    vehLightID->emplace( "DEATHBIKE3", 1 );
    vehLightID->emplace( "IMPALER4", 2 );
    vehLightID->emplace( "SLAMVAN4", 1 );
    vehLightID->emplace( "SLAMVAN5", 1 );
    vehLightID->emplace( "BRUTUS3", 1 );
    vehLightID->emplace( "BRUTUS2", 1 );
    vehLightID->emplace( "BRUTUS", 1 );
    vehLightID->emplace( "DEATHBIKE", 1 );
    vehLightID->emplace( "BRUISER", 0 );
    vehLightID->emplace( "BRUISER2", 0 );
    vehLightID->emplace( "BRUISER3", 0 );
    vehLightID->emplace( "RCBANDITO", 1 );
    vehLightID->emplace( "CERBERUS", 1 );
    vehLightID->emplace( "CERBERUS2", 1 );
    vehLightID->emplace( "IMPALER2", 2 );
    vehLightID->emplace( "MONSTER4", 1 );
    vehLightID->emplace( "MONSTER3", 1 );
    vehLightID->emplace( "TULIP", 1 );
    vehLightID->emplace( "ITALIGTO", 1 );
    vehLightID->emplace( "ISSI4", 1 );
    vehLightID->emplace( "ISSI5", 1 );
    vehLightID->emplace( "SCARAB", 1 );
    vehLightID->emplace( "SCARAB2", 1 );
    vehLightID->emplace( "SCARAB3", 1 );
    vehLightID->emplace( "CLIQUE", 1 );
    vehLightID->emplace( "IMPALER", 2 );
    vehLightID->emplace( "VAMOS", 1 );
    vehLightID->emplace( "IMPERATOR", 2 );
    vehLightID->emplace( "IMPERATOR2", 2 );
    vehLightID->emplace( "IMPERATOR3", 2 );
    vehLightID->emplace( "TOROS", 1 );
    vehLightID->emplace( "SCHLAGEN", 1 );
    vehLightID->emplace( "DEVIANT", 1 );
    vehLightID->emplace( "DOMINATOR4", 1 );
    vehLightID->emplace( "DOMINATOR5", 1 );
    vehLightID->emplace( "ZR380", 1 );
    vehLightID->emplace( "ZR3802", 1 );
    vehLightID->emplace( "ZR3803", 1 );
    vehLightID->emplace( "DEVESTE", 1 );
    vehLightID->emplace( "PARAGON", 1 ); // mpvinewood
    vehLightID->emplace( "PARAGON2", 1 );
    vehLightID->emplace( "JUGULAR", 1 );
    vehLightID->emplace( "RROCKET", 0 );
    vehLightID->emplace( "PEYOTE2", 2 );
    vehLightID->emplace( "NEO", 1 );
    vehLightID->emplace( "KRIEGER", 1 );
    vehLightID->emplace( "S80", 1 );
    vehLightID->emplace( "DYNASTY", 0 );
    vehLightID->emplace( "CARACARA2", 1 );
    vehLightID->emplace( "THRAX", 1 );
    vehLightID->emplace( "NOVAK", 1 );
    vehLightID->emplace( "ZORRUSSO", 1 );
    vehLightID->emplace( "ISSI7", 1 );
    vehLightID->emplace( "LOCUST", 1 );
    vehLightID->emplace( "EMERUS", 1 );
    vehLightID->emplace( "HELLION", 1 );
    vehLightID->emplace( "GAUNTLET3", 2 );
    vehLightID->emplace( "GAUNTLET4", 1 );
    vehLightID->emplace( "NEBULA", 2 );
    vehLightID->emplace( "ZION3", 2 );
    vehLightID->emplace( "DRAFTER", 1 );
    vehLightID->emplace( "RETINUE2", 2 ); // mpheist3
    vehLightID->emplace( "OUTLAW", 1 );
    vehLightID->emplace( "STRYDER", 0 );
    vehLightID->emplace( "YOSEMITE2", 2 );
    vehLightID->emplace( "FORMULA", 0 );
    vehLightID->emplace( "FORMULA2", 0 );
    vehLightID->emplace( "MINITANK", 0 );
    vehLightID->emplace( "SUGOI", 0 );
    vehLightID->emplace( "SULTAN2", 2 );
    vehLightID->emplace( "EVERON", 0 );
    vehLightID->emplace( "REBLA", 0 );
    vehLightID->emplace( "VAGRANT", 1 );
    vehLightID->emplace( "FURIA", 0 );
    vehLightID->emplace( "VSTR", 0 );
    vehLightID->emplace( "KOMODA", 0 );
    vehLightID->emplace( "ASBO", 2 );
    vehLightID->emplace( "ZHABA", 1 );
    vehLightID->emplace( "JB7002", 1 );
    vehLightID->emplace( "KANJO", 2 );
    vehLightID->emplace( "IMORGON", 1 );
    vehLightID->emplace( "OPENWHEEL1", 1 ); // mpsum
    vehLightID->emplace( "COQUETTE4", 0 );
    vehLightID->emplace( "TIGON", 0 );
    vehLightID->emplace( "LANDSTAL2", 0 );
    vehLightID->emplace( "PENUMBRA2", 1 );
    vehLightID->emplace( "YOSEMITE3", 2 );
    vehLightID->emplace( "CLUB", 1 );
    vehLightID->emplace( "SEMINOLE2", 1 );
    vehLightID->emplace( "OPENWHEEL2", 1 );
    vehLightID->emplace( "GAUNTLET5", 2 );
    vehLightID->emplace( "DUKES3", 2 );
    vehLightID->emplace( "GLENDALE2", 1 );
    vehLightID->emplace( "PEYOTE3", 2 );
    vehLightID->emplace( "MANANA2", 2 );
    vehLightID->emplace( "SQUADDIE", 0 ); // mpheist4
    vehLightID->emplace( "ANNIHILATOR", 1 );
    vehLightID->emplace( "VETO", 1 );
    vehLightID->emplace( "VETO2", 1 );
    vehLightID->emplace( "ITALIRSX", 0 );
    vehLightID->emplace( "TOREADOR", 0 );
    vehLightID->emplace( "SLAMTRUCK", 2 );
    vehLightID->emplace( "WEEVIL", 2 );
    vehLightID->emplace( "ALKONOST", 2 );
    vehLightID->emplace( "VETIR", 2 );
    vehLightID->emplace( "PATROLBOAT", 2 );
    vehLightID->emplace( "DINGHY5", 1 );
    vehLightID->emplace( "AVISA", 0 );
    vehLightID->emplace( "BRIOSO2", 2 );
    vehLightID->emplace( "VERUS", 1 );
    vehLightID->emplace( "LONGFIN", 0 );
    vehLightID->emplace( "SEASPARROW2", 1 );
    vehLightID->emplace( "SEASPARROW3", 1 );
    vehLightID->emplace( "WINKY", 0 );
    vehLightID->emplace( "MANCHEZ2", 0 );
    vehLightID->emplace( "KOSATKA", 0 );
    vehLightID->emplace( "FREIGHTCAR2", 0 ); // mptuner
    vehLightID->emplace( "EUROS", 0 );
    vehLightID->emplace( "DOMINATOR8", 2 );
    vehLightID->emplace( "SULTAN3", 2 );
    vehLightID->emplace( "TAILGATER2", 1 );
    vehLightID->emplace( "FUTO2", 2 );
    vehLightID->emplace( "RT3000", 2 );
    vehLightID->emplace( "DOMINATOR7", 0 );
    vehLightID->emplace( "COMET6", 0 );
    vehLightID->emplace( "GROWLER", 0 );
    vehLightID->emplace( "CALICO", 0 );
    vehLightID->emplace( "REMUS", 2 );
    vehLightID->emplace( "ZR350", 2 );
    vehLightID->emplace( "PREVION", 2 );
    vehLightID->emplace( "WARRENER2", 2 );
    vehLightID->emplace( "CYPHER", 0 );
    vehLightID->emplace( "JESTER4", 0 );
    vehLightID->emplace( "VECTRE", 0 );

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
