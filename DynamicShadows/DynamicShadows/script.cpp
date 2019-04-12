#include "script.h"
#include "keyboard.h"

#include <unordered_map>

unsigned int toggleKey       = 0x31;
unsigned int reloadKey       = 0x32;
bool         isOneShadowMode = false;
bool         isModOn         = true;

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
    { 0xFF, 0xFF, 0xCC }, // 0
    { 0x61, 0xA5, 0xFF }, // 1
    { 0xFF, 0xD8, 0x59 }, // 2
    { 0xFF, 0xE8, 0x94 }, // 3
    { 0xD1, 0x24, 0x24 }, // 4
    { 0xFF, 0xCE, 0x1C }, // 5
    { 0x7F, 0xA7, 0xE3 }, // 6
    { 0x24, 0xD1, 0x2C }, // 7
    { 0xFA, 0xEC, 0xC8 }  // 8
};

std::unordered_map<std::string, uint8_t>* vehLightID = nullptr;

__forceinline void update()
{
    const int     vehicleCount    = worldGetAllVehicles(vehsArray, vehsArraySize);
    const Entity  playerID        = PLAYER::PLAYER_PED_ID();
    const Vehicle playerVehicleId = PED::IS_PED_IN_ANY_VEHICLE(playerID, 0)
                                    ? PED::GET_VEHICLE_PED_IS_USING(playerID)
                                    : -1;

    Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(playerID, TRUE);
    playerPos.z += 5.0f;

    for (int i = 0; i < vehicleCount; ++i)
    {
        const register int vehicleID = vehsArray[i];
        if (vehicleID != 0 && ENTITY::DOES_ENTITY_EXIST(vehicleID))
        {
            if (vehicleID == playerVehicleId)
            {
                VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER(vehicleID, 1.0f);
                continue;
            }
            BOOL          highbeamsOn = FALSE;
            BOOL          lightsOn    = FALSE;
            const BOOL    result      = VEHICLE::GET_VEHICLE_LIGHTS_STATE(vehicleID, &lightsOn, &highbeamsOn);
            const BOOL    isRunning   = VEHICLE::GET_IS_VEHICLE_ENGINE_RUNNING(vehicleID);
            const Vector3 vehiclePos  = ENTITY::GET_ENTITY_COORDS(vehicleID, TRUE);
            if (result == TRUE && lightsOn == TRUE && isRunning == TRUE && playerPos.z > vehiclePos.z)
            {
                const Vector3 angle = ENTITY::GET_ENTITY_FORWARD_VECTOR(vehicleID);

                float leftAngle  = angle.z;
                float rightAngle = angle.z;

                float lightDistance   = distance;
                float lightBrightness = brightness;

                if (highbeamsOn == TRUE)
                {
                    lightDistance   *= distanceMultiplier;
                    lightBrightness *= brightnessMultiplier;
                }
                else
                {
                    leftAngle  += leftangle;
                    rightAngle += rightangle;
                }
                VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER(vehicleID, 0.0f);

                const int left  = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(vehicleID, "headlight_l");
                const int right = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(vehicleID, "headlight_r");

                Vector3 leftLightPos  = { 0.0f, 0, 0.0f, 0, 0.0f, 0 };
                Vector3 rightLightPos = { 0.0f, 0, 0.0f, 0, 0.0f, 0 };

                if (left != -1)
                {
                    leftLightPos = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(vehicleID, left);
                }
                if (right != -1)
                {
                    rightLightPos = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(vehicleID, right);
                }

                const Hash  modelHash = ENTITY::GET_ENTITY_MODEL(vehicleID);
                std::string modelName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(modelHash);
                for (auto& c : modelName)
                {
                    c = toupper(c);
                }

                const auto modelID = vehLightID->find(modelName);

                const int colorID = modelID->second % 9;
                const int red     = id[colorID].r;
                const int green   = id[colorID].g;
                const int blue    = id[colorID].b;

                leftLightPos.x  += 0.22f * angle.x;
                leftLightPos.y  += 0.22f * angle.y;
                rightLightPos.x += 0.2f  * angle.x;
                rightLightPos.y += 0.2f  * angle.y;

                const BOOL isLeftDamaged  = VEHICLE::GET_IS_LEFT_VEHICLE_HEADLIGHT_DAMAGED(vehicleID);
                const BOOL isRightDamaged = VEHICLE::GET_IS_RIGHT_VEHICLE_HEADLIGHT_DAMAGED(vehicleID);

                if (isOneShadowMode)
                {
                    if (isLeftDamaged != TRUE || isRightDamaged != TRUE)
                    {
                        radius += 10.0f;
                        if (VEHICLE::IS_THIS_MODEL_A_BIKE(modelHash) || VEHICLE::IS_THIS_MODEL_A_BICYCLE(modelHash))
                        {
                            leftLightPos.x = rightLightPos.x;
                            leftLightPos.y = rightLightPos.y;
                            leftLightPos.z = rightLightPos.z;
                        }
                        else
                        {
                            leftLightPos.x = (leftLightPos.x + rightLightPos.x) / 2;
                            leftLightPos.y = (leftLightPos.y + rightLightPos.y) / 2;
                            leftLightPos.z = (leftLightPos.z + rightLightPos.z) / 2;
                        }
                        GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW(leftLightPos.x, leftLightPos.y, leftLightPos.z, angle.x, angle.y, leftAngle, red, green, blue, lightDistance, lightBrightness, roundness, radius, falloff, vehicleID);
                    }
                }
                else
                {
                    if (isLeftDamaged != TRUE)
                    {
                        GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW(leftLightPos.x, leftLightPos.y, leftLightPos.z, angle.x, angle.y, leftAngle, red, green, blue, lightDistance, lightBrightness, roundness, radius, falloff, vehicleID);
                    }
                    if (isRightDamaged != TRUE)
                    {
                        GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW(rightLightPos.x, rightLightPos.y, rightLightPos.z, angle.x, angle.y, rightAngle, red, green, blue, lightDistance, lightBrightness, roundness, radius, falloff, vehicleID + 1);
                    }
                }
            }
            else
            {
                VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER(vehicleID, 1.0f);
            }
        }
    }
}

unsigned GetPrivateProfileHex(const char* sectionName, const char* keyName, const char* defaultValue, const char* fileName)
{
    char sValue[32];
    GetPrivateProfileString(sectionName, keyName, defaultValue, sValue, 32, fileName);
    return strtol(sValue, nullptr, 16);
}

float GetPrivateProfileFloat(const char* sectionName, const char* keyName, const char* defaultValue, const char* fileName)
{
    char sValue[32];
    GetPrivateProfileString(sectionName, keyName, defaultValue, sValue, 32, fileName);
    return strtof(sValue, nullptr);
}

bool GetPrivateProfileBool(const char* sectionName, const char* keyName, const bool defaultValue, const char* fileName)
{
    return (GetPrivateProfileInt(sectionName, keyName, defaultValue ? 1 : 0, fileName) == 1 ? true : false);
}

void loadConfigFromFile()
{
    char path[MAX_PATH];
    GetModuleFileName(nullptr, path, MAX_PATH);
    for (size_t i = strlen(path); i > 0; --i)
    {
        if (path[i] == '\\')
        {
            path[i] = '\0';
            break;
        }
    }
    strcat_s(path, "\\shadows.ini");

    toggleKey = GetPrivateProfileHex("KEYS", "ToggleKey", "0x31", path);
    reloadKey = GetPrivateProfileHex("KEYS", "ReloadKey", "0x32", path);

    isOneShadowMode = GetPrivateProfileBool("MODES", "OneShadowMode", false, path);

    brightness = GetPrivateProfileFloat("OPTIONS", "LightBrightness", "2.0", path);
    distance = GetPrivateProfileFloat("OPTIONS", "LightDistance", "25.0", path);
    radius = GetPrivateProfileFloat("OPTIONS", "LightRadius", "50.0", path);
    roundness = GetPrivateProfileFloat("OPTIONS", "LightRoundness", "1.0", path);
    falloff = GetPrivateProfileFloat("OPTIONS", "LightFallOff", "20.0", path);
    leftangle = GetPrivateProfileFloat("OPTIONS", "LeftLightAngle", "-0.7", path);
    rightangle = GetPrivateProfileFloat("OPTIONS", "RightLightAngle", "-0.6", path);
    brightnessMultiplier = GetPrivateProfileFloat("OPTIONS", "HighBeamBrightnessMultiplier", "1.5", path);
    distanceMultiplier = GetPrivateProfileFloat("OPTIONS", "HighBeamDistanceMultiplier", "2.0", path);
}

void ScriptMain()
{
    loadConfigFromFile();

    vehLightID = new std::unordered_map<std::string, uint8_t>;

    vehLightID->insert({ "NINEF", 6 });
    vehLightID->insert({ "NINEF2", 6 });
    vehLightID->insert({ "ASEA", 0 });
    vehLightID->insert({ "BOXVILLE2", 0 });
    vehLightID->insert({ "BULLDOZE", 8 }); // BULLDOZER
    vehLightID->insert({ "CHEETAH", 6 });
    vehLightID->insert({ "COGCABRI", 6 }); // COGCABRIO
    vehLightID->insert({ "DUBSTA", 1 });
    vehLightID->insert({ "DUBSTA2", 0 });
    vehLightID->insert({ "EMPEROR", 0 });
    vehLightID->insert({ "ENTITYXF", 6 });
    vehLightID->insert({ "FIRETRUK", 0 });
    vehLightID->insert({ "FQ2", 0 });
    vehLightID->insert({ "INFERNUS", 6 });
    vehLightID->insert({ "JACKAL", 0 });
    vehLightID->insert({ "JOURNEY", 0 });
    vehLightID->insert({ "JB700", 0 });
    vehLightID->insert({ "ORACLE", 1 });
    vehLightID->insert({ "PATRIOT", 1 });
    vehLightID->insert({ "RADI", 1 });
    vehLightID->insert({ "ROMERO", 0 });
    vehLightID->insert({ "STINGER", 2 });
    vehLightID->insert({ "STOCKADE", 0 });
    vehLightID->insert({ "SUPERD", 6 });
    vehLightID->insert({ "TAILGATE", 6 }); // TAILGATER
    vehLightID->insert({ "TORNADO", 2 });
    vehLightID->insert({ "UTILTRUC", 0 }); // UTILLITRUCK
    vehLightID->insert({ "UTILLITRUCK2", 0 });
    vehLightID->insert({ "VOODOO2", 2 });
    vehLightID->insert({ "SCORCHER", 0 });
    vehLightID->insert({ "POLICEB", 6 });
    vehLightID->insert({ "HEXER", 6 });
    vehLightID->insert({ "BUZZARD", 0 });
    vehLightID->insert({ "POLMAV", 0 });
    vehLightID->insert({ "CUBAN800", 0 });
    vehLightID->insert({ "JET", 0 });
    vehLightID->insert({ "TITAN", 0 });
    vehLightID->insert({ "SQUALO", 0 });
    vehLightID->insert({ "MARQUIS", 0 });
    vehLightID->insert({ "FREIGHTCAR", 0 });
    vehLightID->insert({ "FREIGHT", 0 });
    vehLightID->insert({ "FREIGHTCONT1", 0 });
    vehLightID->insert({ "FREIGHTCONT2", 0 });
    vehLightID->insert({ "FREIGHTGRAIN", 0 });
    vehLightID->insert({ "TANKERCAR", 0 });
    vehLightID->insert({ "METROTRAIN", 0 });
    vehLightID->insert({ "TRAILERS", 0 });
    vehLightID->insert({ "TANKER", 0 });
    vehLightID->insert({ "TRAILERLOGS", 0 });
    vehLightID->insert({ "TR2", 0 });
    vehLightID->insert({ "TR3", 0 });
    vehLightID->insert({ "PICADOR", 0 });
    vehLightID->insert({ "POLICEO1", 2 }); // POLICEOLD1
    vehLightID->insert({ "POLICEO2", 2 }); // ADDED
    vehLightID->insert({ "ASTROPE", 0 }); // ASTEROPE
    vehLightID->insert({ "BANSHEE", 6 });
    vehLightID->insert({ "BUFFALO", 1 });
    vehLightID->insert({ "BULLET", 6 });
    vehLightID->insert({ "F620", 6 });
    vehLightID->insert({ "HANDLER", 8 });
    vehLightID->insert({ "RUINER", 2 });
    vehLightID->insert({ "GBURRITO", 0 });
    vehLightID->insert({ "TRACTOR2", 0 });
    vehLightID->insert({ "PENUMBRA", 0 });
    vehLightID->insert({ "SUBMERS", 3 }); // SUBMERSIBLE
    vehLightID->insert({ "DOCKTUG", 8 });
    vehLightID->insert({ "DOCKTRAILER", 0 });
    vehLightID->insert({ "SULTAN", 0 });
    vehLightID->insert({ "DILETTAN", 1 }); // DILETTANTE
    vehLightID->insert({ "FUTO", 2 });
    vehLightID->insert({ "HABANERO", 0 });
    vehLightID->insert({ "INTRUDER", 0 });
    vehLightID->insert({ "LANDSTAL", 1 }); // LANDSTALKER
    vehLightID->insert({ "MINIVAN", 0 });
    vehLightID->insert({ "SCHAFTER2", 6 });
    vehLightID->insert({ "SERRANO", 6 });
    vehLightID->insert({ "MANANA", 2 });
    vehLightID->insert({ "SEASHARK2", 0 });
    vehLightID->insert({ "YOUGA", 0 });
    vehLightID->insert({ "PREMIER", 1 });
    vehLightID->insert({ "SPEEDO", 0 });
    vehLightID->insert({ "WASHINGT", 0 }); // WASHINGTON
    vehLightID->insert({ "ANNIHILATOR", 0 });
    vehLightID->insert({ "BLAZER2", 0 });
    vehLightID->insert({ "CRUISER", 0 });
    vehLightID->insert({ "RAKETRAILER", 0 });
    vehLightID->insert({ "CARGOPLANE", 0 });
    vehLightID->insert({ "DUMP", 8 });
    vehLightID->insert({ "PONY", 2 });
    vehLightID->insert({ "LGUARD", 0 });
    vehLightID->insert({ "SENTINEL", 6 });
    vehLightID->insert({ "SENTINEL2", 6 });
    vehLightID->insert({ "COMET2", 6 });
    vehLightID->insert({ "STINGERG", 2 }); // STINGERGT
    vehLightID->insert({ "INGOT", 2 });
    vehLightID->insert({ "PEYOTE", 2 });
    vehLightID->insert({ "STANIER", 0 });
    vehLightID->insert({ "STRATUM", 2 });
    vehLightID->insert({ "AKUMA", 6 });
    vehLightID->insert({ "CARBON", 6 }); // ADDED
    vehLightID->insert({ "BATI", 6 });
    vehLightID->insert({ "BATI2", 6 });
    vehLightID->insert({ "PCJ", 0 });
    vehLightID->insert({ "DLOADER", 2 });
    vehLightID->insert({ "PRAIRIE", 6 });
    vehLightID->insert({ "DUSTER", 0 });
    vehLightID->insert({ "ISSI2", 1 });
    vehLightID->insert({ "TRAILERS2", 0 });
    vehLightID->insert({ "TVTRAILER", 0 });
    vehLightID->insert({ "CUTTER", 8 });
    vehLightID->insert({ "TRFLAT", 0 });
    vehLightID->insert({ "TORNADO2", 2 });
    vehLightID->insert({ "TORNADO3", 2 });
    vehLightID->insert({ "TRIBIKE", 0 });
    vehLightID->insert({ "TRIBIKE2", 0 });
    vehLightID->insert({ "TRIBIKE3", 0 });
    vehLightID->insert({ "PROPTRAILER", 0 });
    vehLightID->insert({ "BURRITO2", 0 });
    vehLightID->insert({ "DUNE", 0 });
    vehLightID->insert({ "FELTZER2", 6 });
    vehLightID->insert({ "BLISTA", 6 });
    vehLightID->insert({ "BAGGER", 0 });
    vehLightID->insert({ "VOLTIC", 6 });
    vehLightID->insert({ "FUGITIVE", 6 });
    vehLightID->insert({ "FELON", 6 });
    vehLightID->insert({ "PBUS", 0 });
    vehLightID->insert({ "ARMYTRAILER", 0 });
    vehLightID->insert({ "POLICET", 0 });
    vehLightID->insert({ "SPEEDO2", 2 });
    vehLightID->insert({ "FELON2", 6 });
    vehLightID->insert({ "BMX", 0 });
    vehLightID->insert({ "EXEMPLAR", 6 });
    vehLightID->insert({ "FUSILADE", 0 });
    vehLightID->insert({ "BOATTRAILER", 0 });
    vehLightID->insert({ "CAVCADE", 1 }); // CAVALCADE
    vehLightID->insert({ "SURGE", 0 });
    vehLightID->insert({ "BUCCANEE", 2 }); // BUCCANEER
    vehLightID->insert({ "NEMESIS", 6 });
    vehLightID->insert({ "ARMYTANKER", 0 });
    vehLightID->insert({ "ROCOTO", 6 });
    vehLightID->insert({ "STOCKADE3", 0 });
    vehLightID->insert({ "REBEL02", 2 }); // REBEL2
    vehLightID->insert({ "SCHWARZE", 6 }); // SCHWARZER
    vehLightID->insert({ "SCRAP", 8 });
    vehLightID->insert({ "SANDKING", 0 });
    vehLightID->insert({ "SANDKIN2", 0 }); // SANDKING2
    vehLightID->insert({ "CARBONIZ", 6 }); // CARBONIZZARE
    vehLightID->insert({ "RUMPO", 0 });
    vehLightID->insert({ "PRIMO", 0 });
    vehLightID->insert({ "SABREGT", 2 });
    vehLightID->insert({ "REGINA", 2 });
    vehLightID->insert({ "JETMAX", 0 });
    vehLightID->insert({ "TROPIC", 0 });
    vehLightID->insert({ "VIGERO", 0 });
    vehLightID->insert({ "POLICE2", 1 });
    vehLightID->insert({ "STRETCH", 1 });
    vehLightID->insert({ "DINGHY2", 0 });
    vehLightID->insert({ "BOXVILLE", 0 });
    vehLightID->insert({ "LUXOR", 0 });
    vehLightID->insert({ "POLICD3", 6 });
    vehLightID->insert({ "TRAILERS3", 0 });
    vehLightID->insert({ "DOUBLE", 6 });
    vehLightID->insert({ "TRACTOR", 8 });
    vehLightID->insert({ "BIFF", 8 });
    vehLightID->insert({ "DOMINATO", 0 }); // DOMINATOR
    vehLightID->insert({ "HAULER", 8 });
    vehLightID->insert({ "PACKER", 8 });
    vehLightID->insert({ "PHOENIX", 2 });
    vehLightID->insert({ "SADLER", 0 });
    vehLightID->insert({ "SADLER2", 0 });
    vehLightID->insert({ "DAEMON", 0 });
    vehLightID->insert({ "COACH", 0 });
    vehLightID->insert({ "TORNADO4", 2 });
    vehLightID->insert({ "RATLOADER", 2 });
    vehLightID->insert({ "RAPIDGT", 6 });
    vehLightID->insert({ "RAPIDGT2", 6 });
    vehLightID->insert({ "SURANO", 1 });
    vehLightID->insert({ "BFINJECT", 2 }); // BFINJECTION
    vehLightID->insert({ "BISON2", 0 });
    vehLightID->insert({ "BISON3", 0 });
    vehLightID->insert({ "BODHI2", 0 });
    vehLightID->insert({ "BURRITO", 0 });
    vehLightID->insert({ "BURRITO4", 0 });
    vehLightID->insert({ "RUBBLE", 8 });
    vehLightID->insert({ "TIPTRUCK", 8 });
    vehLightID->insert({ "TIPTRUCK2", 8 });
    vehLightID->insert({ "MIXER", 8 });
    vehLightID->insert({ "MIXER2", 8 });
    vehLightID->insert({ "PHANTOM", 8 });
    vehLightID->insert({ "POUNDER", 8 });
    vehLightID->insert({ "BUZZARD2", 0 });
    vehLightID->insert({ "FROGGER", 0 });
    vehLightID->insert({ "AIRTUG", 8 });
    vehLightID->insert({ "BENSON", 8 });
    vehLightID->insert({ "RIPLEY", 8 });
    vehLightID->insert({ "AMBULAN", 0 }); // AMBULANCE
    vehLightID->insert({ "FORK", 8 }); // FORKLIFT
    vehLightID->insert({ "GRANGER", 0 });
    vehLightID->insert({ "PRANGER", 0 });
    vehLightID->insert({ "TRAILERSMALL", 0 });
    vehLightID->insert({ "BARRACKS", 0 });
    vehLightID->insert({ "BARRACKS2", 0 });
    vehLightID->insert({ "CRUSADER", 0 });
    vehLightID->insert({ "UTILLITRUCK3", 0 });
    vehLightID->insert({ "SHERIFF", 0 });
    vehLightID->insert({ "MONROE", 2 });
    vehLightID->insert({ "MULE", 0 });
    vehLightID->insert({ "TACO", 0 });
    vehLightID->insert({ "TRASH", 8 });
    vehLightID->insert({ "DINGHY", 6 });
    vehLightID->insert({ "BLAZER", 0 });
    vehLightID->insert({ "MAVERICK", 0 });
    vehLightID->insert({ "CARGOBOB", 0 });
    vehLightID->insert({ "CARGOBOB3", 0 });
    vehLightID->insert({ "STUNT", 0 });
    vehLightID->insert({ "EMPEROR3", 2 });
    vehLightID->insert({ "CADDY", 0 });
    vehLightID->insert({ "EMPEROR2", 2 });
    vehLightID->insert({ "SURFER2", 2 });
    vehLightID->insert({ "TOWTRUCK", 8 });
    vehLightID->insert({ "TWOTRUCK2", 8 });
    vehLightID->insert({ "BALLER", 1 });
    vehLightID->insert({ "SURFER", 2 });
    vehLightID->insert({ "MAMMATUS", 0 });
    vehLightID->insert({ "RIOT", 0 });
    vehLightID->insert({ "VELUM", 0 });
    vehLightID->insert({ "RANCHERX12", 0 });
    vehLightID->insert({ "CADDY2", 0 });
    vehLightID->insert({ "AIRBUS", 0 });
    vehLightID->insert({ "RENTBUS", 0 }); // RENTALBUS
    vehLightID->insert({ "GRESLEY", 1 });
    vehLightID->insert({ "ZION", 0 });
    vehLightID->insert({ "ZION2", 0 });
    vehLightID->insert({ "RUFFIAN", 6 });
    vehLightID->insert({ "ADDER", 6 });
    vehLightID->insert({ "VACCA", 6 });
    vehLightID->insert({ "BOXVILLE3", 0 });
    vehLightID->insert({ "SUNTRAP" , 0 });
    vehLightID->insert({ "BOBCATXL", 2 });
    vehLightID->insert({ "BURRITO3", 0 });
    vehLightID->insert({ "POLICE4", 1 });
    vehLightID->insert({ "CABLECAR", 0 });
    vehLightID->insert({ "BLIMP", 0 });
    vehLightID->insert({ "BUS", 0 });
    vehLightID->insert({ "DILETTANTE2", 1 });
    vehLightID->insert({ "REBEL01", 0 }); // REBEL
    vehLightID->insert({ "SKYLIFT", 0 });
    vehLightID->insert({ "SHAMAL", 0 });
    vehLightID->insert({ "GRAINTRAILER", 0 });
    vehLightID->insert({ "VADER", 6 });
    vehLightID->insert({ "SHERIFF2", 0 });
    vehLightID->insert({ "BALETRAILER", 0 });
    vehLightID->insert({ "TOURBUS", 0 });
    vehLightID->insert({ "FIXTER", 0 });
    vehLightID->insert({ "ORACLE2", 6 });
    vehLightID->insert({ "BALLER2", 0 });
    vehLightID->insert({ "BUFFALO02", 0 }); // BUFFALO2
    vehLightID->insert({ "CAVALCADE2", 1 });
    vehLightID->insert({ "COQUETTE", 6 });
    vehLightID->insert({ "TRACTOR3", 2 });
    vehLightID->insert({ "GAUNTLET", 6 });
    vehLightID->insert({ "MESA", 0 });
    vehLightID->insert({ "POLICE", 1 });
    vehLightID->insert({ "CARGOBOB2", 0 });
    vehLightID->insert({ "TAXI", 0 });
    vehLightID->insert({ "SANCHEZ", 0 });
    vehLightID->insert({ "FLATBED", 0 });
    vehLightID->insert({ "SEMINOLE", 0 });
    vehLightID->insert({ "MOWER", 0 });
    vehLightID->insert({ "ZTYPE", 0 });
    vehLightID->insert({ "PREDATOR", 1 });
    vehLightID->insert({ "RUMPO2", 0 });
    vehLightID->insert({ "PONY2", 0 });
    vehLightID->insert({ "BJXL", 1 });
    vehLightID->insert({ "CAMPER", 0 });
    vehLightID->insert({ "RANCHERX", 0 }); // RANCHERXL
    vehLightID->insert({ "FAGGIO2", 0 });
    vehLightID->insert({ "LAZER", 0 });
    vehLightID->insert({ "SEASHARK", 0 });
    vehLightID->insert({ "BISON", 1 });
    vehLightID->insert({ "FBI", 1 });
    vehLightID->insert({ "FBI2", 0 });
    vehLightID->insert({ "MULE2", 0 });
    vehLightID->insert({ "RHINO", 0 });
    vehLightID->insert({ "BURRITO5", 0 });
    vehLightID->insert({ "ASES2", 2 });
    vehLightID->insert({ "MESA2", 0 });
    vehLightID->insert({ "MESA3", 0 });
    vehLightID->insert({ "FROGGER2", 0 });
    vehLightID->insert({ "HOTKNIFE", 1 });
    vehLightID->insert({ "ELEGY2", 6 });
    vehLightID->insert({ "KHAMEL", 6 }); // KHAMELION
    vehLightID->insert({ "DUNE2", 7 });
    vehLightID->insert({ "ARMYTRAILER2", 0 });
    vehLightID->insert({ "FREIGHTTRAILER", 0 });
    vehLightID->insert({ "TR4", 0 });
    vehLightID->insert({ "BLAZER03", 0 }); // BLAZER3
    vehLightID->insert({ "SANCHEZ01", 0 }); // ADDED
    vehLightID->insert({ "SANCHEZ02", 0 }); // SANCHEZ2
    vehLightID->insert({ "VERLIER", 1 }); // mpapartment // VERLIERER2
    vehLightID->insert({ "MAMBA", 2 });
    vehLightID->insert({ "NITESHAD", 2 }); // NIGHTSHADE
    vehLightID->insert({ "COG55", 6 });
    vehLightID->insert({ "COG552", 6 });
    vehLightID->insert({ "COGNOSC", 6 }); // COGNOSCENTI
    vehLightID->insert({ "COGNOSC2", 6 }); // COGNOSCENTI2
    vehLightID->insert({ "SCHAFTER3", 6 });
    vehLightID->insert({ "SCHAFTER4", 6 });
    vehLightID->insert({ "SCHAFTER5", 6 });
    vehLightID->insert({ "SCHAFTER6", 6 });
    vehLightID->insert({ "LIMO2", 6 });
    vehLightID->insert({ "BALLER3", 0 });
    vehLightID->insert({ "BALLER4", 0 });
    vehLightID->insert({ "BALLER5", 0 });
    vehLightID->insert({ "BALLER6", 0 });
    vehLightID->insert({ "SEASHARK3", 0 });
    vehLightID->insert({ "DINGHY4", 0 });
    vehLightID->insert({ "TROPIC2", 0 });
    vehLightID->insert({ "SPEEDER2", 0 });
    vehLightID->insert({ "TORO2", 0 });
    vehLightID->insert({ "CARGOBOB4", 0 });
    vehLightID->insert({ "SUPERVOLITO", 0 });
    vehLightID->insert({ "SUPERVOLITO2", 0 });
    vehLightID->insert({ "VALKYRIE2", 0 });
    vehLightID->insert({ "BIFTA", 0 }); // mpbeach
    vehLightID->insert({ "SPEEDER", 0 });
    vehLightID->insert({ "KALAHARI", 0 });
    vehLightID->insert({ "PARADISE", 0 });
    vehLightID->insert({ "TORNADO6", 2 }); //mpbiker
    vehLightID->insert({ "RAPTOR", 6 });
    vehLightID->insert({ "VORTEX", 6 });
    vehLightID->insert({ "AVARUS", 6 });
    vehLightID->insert({ "SANCTUS", 4 });
    vehLightID->insert({ "FAGGIO3", 0 });
    vehLightID->insert({ "FAGGIO", 0 });
    vehLightID->insert({ "FAGGION", 0 }); // ADDED
    vehLightID->insert({ "HAKUCHOU2", 1 });
    vehLightID->insert({ "NIGHTBLADE", 0 });
    vehLightID->insert({ "YOUGA2", 6 });
    vehLightID->insert({ "CHIMERA", 0 });
    vehLightID->insert({ "ESSKEY", 0 });
    vehLightID->insert({ "ZOMBIEA", 0 });
    vehLightID->insert({ "WOLFSBANE", 0 });
    vehLightID->insert({ "DAEMON2", 0 });
    vehLightID->insert({ "SHOTARO", 0 });
    vehLightID->insert({ "RATBIKE", 0 });
    vehLightID->insert({ "ZOMBIEB", 0 });
    vehLightID->insert({ "DEFILER", 0 });
    vehLightID->insert({ "MANCHEZ", 0 });
    vehLightID->insert({ "BLAZER4", 0 });
    vehLightID->insert({ "JESTER", 0 }); // mpbusiness
    vehLightID->insert({ "TURISMOR", 0 });
    vehLightID->insert({ "ALPHA", 6 });
    vehLightID->insert({ "VESTRA", 0 });
    vehLightID->insert({ "ZENTORNO", 0 }); // mpbusiness2
    vehLightID->insert({ "MASSACRO", 6 });
    vehLightID->insert({ "HUNTLEY", 6 });
    vehLightID->insert({ "THRUST", 6 });
    vehLightID->insert({ "SLAMVAN", 2 }); // mpchristmas2
    vehLightID->insert({ "RLOADER2", 2 }); // RATLOADER2
    vehLightID->insert({ "JESTER2", 0 });
    vehLightID->insert({ "MASSACRO2", 6 });
    vehLightID->insert({ "NIMBUS", 0 }); // mpexecutive
    vehLightID->insert({ "XLS", 1 });
    vehLightID->insert({ "XLS2", 1 });
    vehLightID->insert({ "SEVEN70", 6 });
    vehLightID->insert({ "REAPER", 6 });
    vehLightID->insert({ "FMJ", 6 });
    vehLightID->insert({ "PFISTER811", 6 });
    vehLightID->insert({ "BESTIAGTS", 6 });
    vehLightID->insert({ "BRICKADE", 0 });
    vehLightID->insert({ "RUMPO3", 0 });
    vehLightID->insert({ "PROTOTIPO", 6 });
    vehLightID->insert({ "WINDSOR2", 6 });
    vehLightID->insert({ "VOLATUS", 0 });
    vehLightID->insert({ "TUG", 0 });
    vehLightID->insert({ "TRAILERS4", 0 }); // mpgunrunning
    vehLightID->insert({ "XA21", 0 });
    vehLightID->insert({ "VAGNER", 6 });
    vehLightID->insert({ "CADDY3", 0 });
    vehLightID->insert({ "PHANTOM3", 1 });
    vehLightID->insert({ "NIGHTSHARK", 1 });
    vehLightID->insert({ "CHEETAH2", 6 });
    vehLightID->insert({ "TORERO", 6 });
    vehLightID->insert({ "HAULER2", 8 });
    vehLightID->insert({ "TRAILERLARGE", 6 });
    vehLightID->insert({ "TECHNICAL3", 0 });
    vehLightID->insert({ "TAMPA3", 2 });
    vehLightID->insert({ "INSURGENT3", 1 });
    vehLightID->insert({ "APC", 2 });
    vehLightID->insert({ "HALFTRACK", 2 });
    vehLightID->insert({ "DUNE3", 0 });
    vehLightID->insert({ "TRAILERSMALL2", 0 });
    vehLightID->insert({ "ARDENT", 6 });
    vehLightID->insert({ "OPPRESSOR", 0 });
    vehLightID->insert({ "LURCHER", 2 }); // mphalloween
    vehLightID->insert({ "BTYPE2", 2 });
    vehLightID->insert({ "MULE3", 0 }); // mpheist
    vehLightID->insert({ "TRASH2", 8 });
    vehLightID->insert({ "VELUM2", 0 });
    vehLightID->insert({ "TANKER2", 0 });
    vehLightID->insert({ "ENDURO", 1 });
    vehLightID->insert({ "SAVAGE", 1 });
    vehLightID->insert({ "CASCO", 0 });
    vehLightID->insert({ "TECHNICAL", 0 });
    vehLightID->insert({ "INSURGENT", 0 });
    vehLightID->insert({ "INSURGENT2", 0 });
    vehLightID->insert({ "HYDRA", 0 });
    vehLightID->insert({ "BOXVILLE4", 0 });
    vehLightID->insert({ "GBURRITO2", 0 });
    vehLightID->insert({ "GUARDIAN", 6 });
    vehLightID->insert({ "DINGHY3", 0 });
    vehLightID->insert({ "LECTRO", 6 });
    vehLightID->insert({ "KURUMA", 6 });
    vehLightID->insert({ "KURUMA2", 6 });
    vehLightID->insert({ "BARRCAKS3", 0 });
    vehLightID->insert({ "VALKYRIE", 0 });
    vehLightID->insert({ "SLAMVAN2", 0 });
    vehLightID->insert({ "RHAPSODY", 0 }); // mphipster
    vehLightID->insert({ "WARRENER", 2 });
    vehLightID->insert({ "BLADE", 2 });
    vehLightID->insert({ "GLENDALE", 5 });
    vehLightID->insert({ "PANTO", 0 });
    vehLightID->insert({ "DUBSTA3", 1 });
    vehLightID->insert({ "PIGALLE", 5 });
    vehLightID->insert({ "NERO", 0 }); // mpimportexport
    vehLightID->insert({ "NERO2", 0 });
    vehLightID->insert({ "ELEGY", 2 });
    vehLightID->insert({ "ITALIGTB", 0 });
    vehLightID->insert({ "ITALIGTB2", 0 });
    vehLightID->insert({ "TEMPESTA", 0 });
    vehLightID->insert({ "DIABLOUS", 1 });
    vehLightID->insert({ "SPECTER", 6 });
    vehLightID->insert({ "SPECTER2", 6 });
    vehLightID->insert({ "DIABLOUS2", 1 });
    vehLightID->insert({ "COMET3", 6 });
    vehLightID->insert({ "BLAZER5", 0 });
    vehLightID->insert({ "DUNE4", 1 });
    vehLightID->insert({ "DUNE5", 1 });
    vehLightID->insert({ "RUINER2", 2 });
    vehLightID->insert({ "VOLTIC2", 6 });
    vehLightID->insert({ "PHANTOM2", 1 });
    vehLightID->insert({ "BOXVILLE5", 1 });
    vehLightID->insert({ "WASTLNDR", 0 }); // WESTERLANDER
    vehLightID->insert({ "TECHNICAL2", 1 });
    vehLightID->insert({ "PENETRATOR", 6 });
    vehLightID->insert({ "FCR", 1 });
    vehLightID->insert({ "FCR2", 1 });
    vehLightID->insert({ "RUINER3", 1 });
    vehLightID->insert({ "MONSTER", 1 }); // mpindependence
    vehLightID->insert({ "SOVEREIGN", 1 });
    vehLightID->insert({ "SULTANRS", 6 }); // mpjanuary2016
    vehLightID->insert({ "BANSHEE2", 6 });
    vehLightID->insert({ "BUCCANEE2", 2 }); // mplowrider // BUCCANEER2
    vehLightID->insert({ "VOODOO", 2 });
    vehLightID->insert({ "FACTION", 2 });
    vehLightID->insert({ "FACTION2", 2 });
    vehLightID->insert({ "MOONBEAM", 2 });
    vehLightID->insert({ "MOONBEAM2", 2 });
    vehLightID->insert({ "PRIMO2", 2 });
    vehLightID->insert({ "CHINO2", 6 });
    vehLightID->insert({ "FACTION3", 2 }); // mplowrider2
    vehLightID->insert({ "MINIVAN2", 2 });
    vehLightID->insert({ "SABREGT2", 2 });
    vehLightID->insert({ "SLAMVAN3", 2 });
    vehLightID->insert({ "TORNADO5", 2 });
    vehLightID->insert({ "VIRGO2", 2 });
    vehLightID->insert({ "VIRGO3", 2 });
    vehLightID->insert({ "INNOVATION", 1 }); // mplts
    vehLightID->insert({ "HAKUCHOU", 1 });
    vehLightID->insert({ "FURORE", 0 }); // FUROREGT
    vehLightID->insert({ "SWIFT2", 0 }); // mpluxe
    vehLightID->insert({ "LUXOR2", 0 });
    vehLightID->insert({ "FELTZER3", 2 });
    vehLightID->insert({ "OSIRIS", 6 });
    vehLightID->insert({ "VIRGO", 2 });
    vehLightID->insert({ "WINDSOR", 6 });
    vehLightID->insert({ "COQUETTE3", 2 }); // mpluxe2
    vehLightID->insert({ "VINDICATOR", 0 });
    vehLightID->insert({ "T20", 0 });
    vehLightID->insert({ "BRAWLER", 6 });
    vehLightID->insert({ "TORO", 6 });
    vehLightID->insert({ "CHINO", 6 });
    vehLightID->insert({ "MILJET", 0 }); // mppilot
    vehLightID->insert({ "BESRA", 1 });
    vehLightID->insert({ "COQUETTE2", 2 });
    vehLightID->insert({ "SWIFT", 0 });
    vehLightID->insert({ "VIGILANTE", 6 }); // mpsmuggler
    vehLightID->insert({ "BOMBUSHKA", 1 });
    vehLightID->insert({ "HOWARD", 1 });
    vehLightID->insert({ "ALPHAZ1", 1 });
    vehLightID->insert({ "SEABREEZE", 1 });
    vehLightID->insert({ "NOKOTA", 1 });
    vehLightID->insert({ "MOLOTOK", 1 });
    vehLightID->insert({ "STARLING", 1 });
    vehLightID->insert({ "HAVOK", 1 });
    vehLightID->insert({ "TULA", 1 });
    vehLightID->insert({ "MICROLIGHT", 1 });
    vehLightID->insert({ "HUNTER", 1 });
    vehLightID->insert({ "ROGUE", 1 });
    vehLightID->insert({ "PYRO", 1 });
    vehLightID->insert({ "RAPIDGT3", 2 });
    vehLightID->insert({ "MOGUL", 1 });
    vehLightID->insert({ "RETINUE", 2 });
    vehLightID->insert({ "CYCLONE", 6 });
    vehLightID->insert({ "VISIONE", 1 });
    vehLightID->insert({ "TURISMO2", 6 }); // mpspecialraces
    vehLightID->insert({ "INFERNUS2", 6 });
    vehLightID->insert({ "GP1", 6 });
    vehLightID->insert({ "RUSTON", 6 });
    vehLightID->insert({ "GARGOYLE", 0 }); // mpstunt
    vehLightID->insert({ "OMNIS", 0 });
    vehLightID->insert({ "SHEAVA", 0 });
    vehLightID->insert({ "TYRUS", 0 });
    vehLightID->insert({ "LE7B", 0 });
    vehLightID->insert({ "LYNX", 0 });
    vehLightID->insert({ "TROPOS", 2 });
    vehLightID->insert({ "TAMPA2", 2 });
    vehLightID->insert({ "BRIOSO", 0 });
    vehLightID->insert({ "BF400", 1 });
    vehLightID->insert({ "CLIFFHANGER", 1 });
    vehLightID->insert({ "CONTENDER", 0 });
    vehLightID->insert({ "E109", 0 }); // ADDED
    vehLightID->insert({ "TROPHY", 6 }); // TROPHYTRUCK
    vehLightID->insert({ "TROPHY2", 6 }); // TROPHYTRUCK2
    vehLightID->insert({ "RALLYTRUCK", 0 });
    vehLightID->insert({ "ROOSEVELT", 0 }); // mpvalentines // BTYPE
    vehLightID->insert({ "ROOSEVELT2", 2 }); // mpvalentines2 // BTYPE3
    vehLightID->insert({ "TAMPA", 2 }); // mpxmas_604490
    vehLightID->insert({ "SUBMERS2", 3 }); // spupgrade // SUBMERSIBLE2
    vehLightID->insert({ "MARSHALL", 1 });
    vehLightID->insert({ "BLIMP2", 0 });
    vehLightID->insert({ "DUKES", 2 });
    vehLightID->insert({ "DUKES2", 2 });
    vehLightID->insert({ "BUFFALO3", 0 });
    vehLightID->insert({ "DOMINATO2", 0 }); // DOMINATOR2
    vehLightID->insert({ "GAUNTLET2", 6 });
    vehLightID->insert({ "STALION", 2 });
    vehLightID->insert({ "STALION2", 2 });
    vehLightID->insert({ "BLISTA2", 2 });
    vehLightID->insert({ "BLISTA3", 2 });
    vehLightID->insert({ "ADMIRAL", 0 }); // IV Pack
    vehLightID->insert({ "ANGEL", 0 });
    vehLightID->insert({ "APC2", 0 });
    vehLightID->insert({ "BLADE2", 1 });
    vehLightID->insert({ "BOBCAT", 2 });
    vehLightID->insert({ "BODHI", 0 });
    vehLightID->insert({ "BOXVILLE6", 0 });
    vehLightID->insert({ "BRICKADE2", 0 });
    vehLightID->insert({ "BUCCANEER3", 0 });
    vehLightID->insert({ "BUS2", 0 });
    vehLightID->insert({ "CABBY", 0 });
    vehLightID->insert({ "CHAVOS", 0 });
    vehLightID->insert({ "CHAVOS2", 0 });
    vehLightID->insert({ "CHEETAH3", 1 });
    vehLightID->insert({ "CONTENDER2", 0 });
    vehLightID->insert({ "COQUETTE4", 0 });
    vehLightID->insert({ "DF8", 6 });
    vehLightID->insert({ "DIABOLUS", 0 });
    vehLightID->insert({ "DOUBLE2", 6 });
    vehLightID->insert({ "ESPERANTO", 0 });
    vehLightID->insert({ "EMPEROR4", 0 });
    vehLightID->insert({ "FELTZER", 6 });
    vehLightID->insert({ "FEROCI", 0 });
    vehLightID->insert({ "FEROCI2", 0 });
    vehLightID->insert({ "FLATBED2", 0 });
    vehLightID->insert({ "FLOATER", 1 });
    vehLightID->insert({ "FORTUNE", 0 });
    vehLightID->insert({ "FREEWAY", 0 });
    vehLightID->insert({ "FUTO2", 2 });
    vehLightID->insert({ "FXT", 1 });
    vehLightID->insert({ "GHAWAR", 0 });
    vehLightID->insert({ "HAKUMAI", 0 });
    vehLightID->insert({ "HAKUCHOU3", 0 });
    vehLightID->insert({ "HELLFURY", 0 });
    vehLightID->insert({ "HUNTLEY2", 0 });
    vehLightID->insert({ "INTERC", 6 }); // INTERCEPTOR
    vehLightID->insert({ "JB7002", 2 });
    vehLightID->insert({ "LOKUS", 0 });
    vehLightID->insert({ "LYCAN", 0 });
    vehLightID->insert({ "LYCAN2", 0 });
    vehLightID->insert({ "MARBELLE", 2 });
    vehLightID->insert({ "MERIT", 0 });
    vehLightID->insert({ "MRTASTY", 0 });
    vehLightID->insert({ "NIGHTBLADE2", 0 });
    vehLightID->insert({ "NOOSE", 0 });
    vehLightID->insert({ "NRG900", 0 });
    vehLightID->insert({ "NSTOCKADE", 0 });
    vehLightID->insert({ "PACKER2", 0 });
    vehLightID->insert({ "PERENNIAL", 0 });
    vehLightID->insert({ "PERENNIAL2", 0 });
    vehLightID->insert({ "PHOENIX2", 2 });
    vehLightID->insert({ "PINNACLE", 0 });
    vehLightID->insert({ "PMP600", 0 });
    vehLightID->insert({ "POLICE6", 0 });
    vehLightID->insert({ "POLICE7", 1 });
    vehLightID->insert({ "POLICE8", 0 });
    vehLightID->insert({ "POLPATRIOT", 1 });
    vehLightID->insert({ "PREMIER2", 0 });
    vehLightID->insert({ "PRES", 6 });
    vehLightID->insert({ "PRES2", 1 });
    vehLightID->insert({ "PSTOCKADE", 0 });
    vehLightID->insert({ "RANCHER", 0 });
    vehLightID->insert({ "REBLA", 3 });
    vehLightID->insert({ "REEFER", 3 });
    vehLightID->insert({ "REGINA2", 0 });
    vehLightID->insert({ "REGINA3", 2 });
    vehLightID->insert({ "REVENANT", 0 });
    vehLightID->insert({ "ROM", 0 });
    vehLightID->insert({ "SABRE", 2 });
    vehLightID->insert({ "SABRE2", 2 });
    vehLightID->insert({ "SCHAFTER", 6 });
    vehLightID->insert({ "SCHAFTERGTR", 6 });
    vehLightID->insert({ "SENTINEL4", 0 });
    vehLightID->insert({ "SMUGGLER", 1 });
    vehLightID->insert({ "SOLAIR", 0 });
    vehLightID->insert({ "SOVEREIGN2", 0 });
    vehLightID->insert({ "STANIER2", 0 });
    vehLightID->insert({ "STEED", 0 });
    vehLightID->insert({ "STRATUM2", 2 });
    vehLightID->insert({ "STRETCH2", 0 });
    vehLightID->insert({ "STRETCH3", 6 });
    vehLightID->insert({ "SULTAN2", 0 });
    vehLightID->insert({ "SUPERD2", 6 });
    vehLightID->insert({ "SUPERGT", 0 });
    vehLightID->insert({ "TAXI2", 2 });
    vehLightID->insert({ "TAXI3", 0 });
    vehLightID->insert({ "TOURMAV", 0 });
    vehLightID->insert({ "TURISMO", 0 });
    vehLightID->insert({ "TYPHOON", 0 });
    vehLightID->insert({ "URANUS", 0 });
    vehLightID->insert({ "VIGERO2", 2 });
    vehLightID->insert({ "VINCENT", 0 });
    vehLightID->insert({ "VIOLATOR", 1 });
    vehLightID->insert({ "VOODOO3", 2 });
    vehLightID->insert({ "WAYFARER", 0 });
    vehLightID->insert({ "WILLARD", 0 });
    vehLightID->insert({ "WOLFSBANE2", 0 });
    vehLightID->insert({ "YANKEE", 8 });
    vehLightID->insert({ "TANKEE2", 8 });
    vehLightID->insert({ "COMET5", 6 }); // mpchristmas2017
    vehLightID->insert({ "RAIDEN", 6 });
    vehLightID->insert({ "VISERIS", 0 });
    vehLightID->insert({ "RIATA", 1 });
    vehLightID->insert({ "KAMACHO", 1 });
    vehLightID->insert({ "SC1", 6 });
    vehLightID->insert({ "AUTARCH", 6 });
    vehLightID->insert({ "SAVESTRA", 6 });
    vehLightID->insert({ "GT500", 0 });
    vehLightID->insert({ "NEON", 6 });
    vehLightID->insert({ "YOSEMITE", 2 });
    vehLightID->insert({ "HERMES", 2 });
    vehLightID->insert({ "HUSTLER", 2 });
    vehLightID->insert({ "SENTINEL3", 2 });
    vehLightID->insert({ "Z190", 1 });
    vehLightID->insert({ "KHANJALI", 1 });
    vehLightID->insert({ "BARRAGE", 1 });
    vehLightID->insert({ "VOLATOL", 2 });
    vehLightID->insert({ "AKULA", 1 });
    vehLightID->insert({ "AVENGER", 1 });
    vehLightID->insert({ "AVENGER2", 1 });
    vehLightID->insert({ "DELUXO", 1 });
    vehLightID->insert({ "STROMBERG", 1 });
    vehLightID->insert({ "CHERNOBOG", 2 });
    vehLightID->insert({ "RIOT2", 1 });
    vehLightID->insert({ "THRUSTER", 1 });
    vehLightID->insert({ "STREITER", 6 });
    vehLightID->insert({ "REVOLTER", 1 });
    vehLightID->insert({ "PARIAH", 0 });
    vehLightID->insert({ "CARACARA", 1 }); // mpassault
    vehLightID->insert({ "SEASPARROW", 6 });
    vehLightID->insert({ "ENTITY2", 6 });
    vehLightID->insert({ "JESTER3", 1 });
    vehLightID->insert({ "TYRANT", 6 });
    vehLightID->insert({ "DOMINATOR3", 6 });
    vehLightID->insert({ "HOTRING", 6 });
    vehLightID->insert({ "FLASHGT", 6 });
    vehLightID->insert({ "TEZERACT", 1 });
    vehLightID->insert({ "ELLIE", 6 });
    vehLightID->insert({ "MICHELLI", 6 });
    vehLightID->insert({ "GB200", 1 });
    vehLightID->insert({ "ISSI3", 1 });
    vehLightID->insert({ "TAIPAN", 0 });
    vehLightID->insert({ "FAGALOA", 2 });
    vehLightID->insert({ "CHEBUREK", 2 });
    vehLightID->insert({ "STAFFORD", 2 }); // mpbattle
    vehLightID->insert({ "SCRAMJET", 1 });
    vehLightID->insert({ "STRIKEFORCE", 0 });
    vehLightID->insert({ "TERBYTE", 1 });
    vehLightID->insert({ "PBUS2", 0 });
    vehLightID->insert({ "POUNDER2", 8 });
    vehLightID->insert({ "FREECRAWLER", 6 });
    vehLightID->insert({ "MULE4", 1 });
    vehLightID->insert({ "BLIMP3", 0 });
    vehLightID->insert({ "MENACER", 1 });
    vehLightID->insert({ "SWINGER", 0 });
    vehLightID->insert({ "PATRIOT2", 1 });
    vehLightID->insert({ "OPPRESSOR2", 0 });
    vehLightID->insert({ "IMPALER3", 2 }); // mpchristmas2018
    vehLightID->insert({ "MONSTER5", 1 });
    vehLightID->insert({ "SLAMVAN6", 1 });
    vehLightID->insert({ "ISSI6", 1 });
    vehLightID->insert({ "CERBERUS3", 6 });
    vehLightID->insert({ "DEATHBIKE2", 0 });
    vehLightID->insert({ "DOMINATOR6", 1 });
    vehLightID->insert({ "DEATHBIKE3", 1 });
    vehLightID->insert({ "IMPALER4", 2 });
    vehLightID->insert({ "SLAMVAN4", 1 });
    vehLightID->insert({ "SLAMVAN5", 1 });
    vehLightID->insert({ "BRUTUS3", 1 });
    vehLightID->insert({ "BRUTUS2", 1 });
    vehLightID->insert({ "BRUTUS", 1 });
    vehLightID->insert({ "DEATHBIKE", 1 });
    vehLightID->insert({ "BRUISER", 0 });
    vehLightID->insert({ "BRUISER2", 0 });
    vehLightID->insert({ "BRUISER3", 0 });
    vehLightID->insert({ "RCBANDITO", 1 });
    vehLightID->insert({ "CERBERUS", 6 });
    vehLightID->insert({ "CERBERUS2", 6 });
    vehLightID->insert({ "IMPALER2", 2 });
    vehLightID->insert({ "MONSTER4", 1 });
    vehLightID->insert({ "MONSTER3", 1 });
    vehLightID->insert({ "TULIP", 1 });
    vehLightID->insert({ "ITALIGTO", 6 });
    vehLightID->insert({ "ISSI4", 1 });
    vehLightID->insert({ "ISSI5", 1 });
    vehLightID->insert({ "SCARAB", 1 });
    vehLightID->insert({ "SCARAB2", 1 });
    vehLightID->insert({ "SCARAB3", 1 });
    vehLightID->insert({ "CLIQUE", 1 });
    vehLightID->insert({ "IMPALER", 2 });
    vehLightID->insert({ "VAMOS", 1 });
    vehLightID->insert({ "IMPERATOR", 2 });
    vehLightID->insert({ "IMPERATOR2", 2 });
    vehLightID->insert({ "IMPERATOR3", 2 });
    vehLightID->insert({ "TOROS", 6 });
    vehLightID->insert({ "SCHLAGEN", 6 });
    vehLightID->insert({ "DEVIANT", 1 });
    vehLightID->insert({ "DOMINATOR4", 1 });
    vehLightID->insert({ "DOMINATOR5", 1 });
    vehLightID->insert({ "ZR380", 1 });
    vehLightID->insert({ "ZR3802", 1 });
    vehLightID->insert({ "ZR3803", 1 });
    vehLightID->insert({ "DEVESTE", 1 });

    while (true)
    {
        if (IsKeyJustUp(reloadKey))
        {
            loadConfigFromFile();
        }
        if (IsKeyJustUp(toggleKey))
        {
            isModOn = !isModOn;
        }
        if (isModOn)
        {
            update();
        }
        else
        {
            UnloadScript();
        }
        WAIT(0);
    }
}

void UnloadScript()
{
    for (unsigned i = 0; i < vehsArraySize; ++i)
    {
        const int vehicleID = vehsArray[i];
        if (vehicleID != 0 && ENTITY::DOES_ENTITY_EXIST(vehicleID))
        {
            VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER(vehicleID, 1.0f);
        }
    }

    if (vehLightID != nullptr)
    {
        delete vehLightID;
        vehLightID = nullptr;
    }
}
