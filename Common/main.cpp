/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#include "..\inc\main.h"
#include "inc\script.h"
#include "inc\keyboard.h"

BOOL APIENTRY DllMain( HMODULE hInstance, DWORD reason, LPVOID lpReserved )
{
    (void) lpReserved;

    switch( reason )
    {
        case DLL_PROCESS_ATTACH:
            scriptRegister( hInstance, ScriptMain );
#ifndef WEATHER_DETECTOR
            keyboardHandlerRegister( OnKeyboardMessage );
#endif
            break;
        case DLL_PROCESS_DETACH:
            UnloadScript();
            scriptUnregister( hInstance );
#ifndef WEATHER_DETECTOR
            keyboardHandlerUnregister( OnKeyboardMessage );
#endif
            break;
    }
    return TRUE;
}
