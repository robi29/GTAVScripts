/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#include "script.h"
#include "keyboard.h"
//#include <cstdio>

/*char text[128];

void update_status_text()
{
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.55, 0.55);
    UI::SET_TEXT_COLOUR(255, 255, 255, 255);
    UI::SET_TEXT_WRAP(0.0f, 1.0f);
    UI::SET_TEXT_DROPSHADOW(3, 0, 0, 0, 0);
    UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
    UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
    UI::END_TEXT_COMMAND_DISPLAY_TEXT(0.5, 0.5);
}*/


float pedBlur = 0.1f;
float vehBlur = 1.0f;
float vehStart = 0.0f;
bool enabledVeh = true;
bool enabledPed = true;
bool enabledMod = true;

__forceinline void update()
{
    //sprintf_s(text, "index: %d\n", GRAPHICS::GET_TIMECYCLE_MODIFIER_INDEX());
    if (STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS() || !CUTSCENE::HAS_CUTSCENE_FINISHED())
    {
        //sprintf_s(text, "%s1\n", text);
        return;
    }

    //sprintf_s(text, "jo %d", (int)id);
    
    if (GRAPHICS::GET_TIMECYCLE_MODIFIER_INDEX() == -1)
    {
        GRAPHICS::SET_TIMECYCLE_MODIFIER("motionblur");
    }

    const Player player = PLAYER::PLAYER_ID();
    const Ped playerPed = PLAYER::PLAYER_PED_ID();

    if (!enabledMod || !ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player) || PLAYER::IS_PLAYER_DEAD(player) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
    {
        //sprintf_s(text, "%s2\n", text);
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(0.0);
        return;
    }

    if (!PED::IS_PED_IN_ANY_VEHICLE(playerPed, FALSE))
    {
        //sprintf_s(text, "%s3\n", text);
        if (enabledPed)
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(pedBlur);
        }
        else
        {
            GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(0.0);
        }
        return;
    }

    if (CAM::GET_FOLLOW_VEHICLE_CAM_VIEW_MODE() == 4 || PED::IS_PED_IN_FLYING_VEHICLE(playerPed) || !enabledVeh)
    {
        //sprintf_s(text, "%s4\n", text);
        GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(0.0);
        return;
    }

    //int cameramode2 = int(CAM::GET_FOLLOW_VEHICLE_CAM_ZOOM_LEVEL());
    //int ped = CAM::GET_FOLLOW_PED_CAM_VIEW_MODE();
    //int ped2 = CAM::GET_FOLLOW_PED_CAM_ZOOM_LEVEL();
    //BOOL costam = CAM::IS_FOLLOW_VEHICLE_CAM_ACTIVE();

    //BOOL mode = CAM::GET_FOLLOW_PED_CAM_VIEW_MODE();

    //int level = CAM::GET_FOLLOW_PED_CAM_ZOOM_LEVEL();

    //sprintf_s(text, "camera: %d\nin heli/plane: %d\nmode: %d\nzoom level: %d", cameramode, (int)inFlyingVehicle, (int)mode, level);
    //set_status_text(text);

    //sprintf_s(text, "%s5\n", text);
    //set_status_text(text);

    float speed = ENTITY::GET_ENTITY_SPEED(PED::GET_VEHICLE_PED_IS_USING(playerPed));

    speed -= vehStart / 3.6f;
    speed /= 150.0;
    speed *= vehBlur;

    if (speed < 0.0)
    {
        speed = 0.0;
    }
    else if (speed > 1.0)
    {
        speed = 1.0;
    }

    GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(speed);
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

void loadConfig()
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
    strcat_s(path, "\\advancedblur.ini");

    enabledPed = GetPrivateProfileBool("PEDBLUR", "Enabled", true, path);
    pedBlur = GetPrivateProfileFloat("PEDBLUR", "Strength", "0.1", path);

    enabledVeh = GetPrivateProfileBool("VEHICLEBLUR", "Enabled", true, path);
    vehBlur = GetPrivateProfileFloat("VEHICLEBLUR", "Strength", "1.0", path);
    vehStart = GetPrivateProfileFloat("VEHICLEBLUR", "StartVelocity", "0.0", path);
}

void ScriptMain()
{
    loadConfig();

    GRAPHICS::SET_TIMECYCLE_MODIFIER("motionblur");
    GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(0.0);

    while (true)
    {
        if (IsKeyJustUp(VK_DELETE))
        {
            enabledMod = !enabledMod;
            if (enabledMod)
            {
                loadConfig();
            }
        }
        update();
        //update_status_text();
        WAIT(100);
    }
}

void UnloadScript()
{
    GRAPHICS::SET_TIMECYCLE_MODIFIER("motionblur");
    GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(0.0);
}