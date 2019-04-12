//ENBSeries SDK
//author: Boris Vorontsov, 2018
//Main header file with parameters and functions definitions
#ifndef	ENBSERIES_H
#define	ENBSERIES_H

//Version of this SDK. 1000 means 1.0, 1001 means 1.01, etc
#define ENBSDKVERSION			1001

//Unique identifier of the game. SDK may differ for games, so always test it to avoid bugs when users copy files to wrong game
#define ENBGAMEID_GTA5		0x10000021



#ifdef EDECLAREEXPORT
	#ifndef __cplusplus
	#define EEXPORT				_declspec(dllexport)
	#else
	#define EEXPORT	extern	"C"	_declspec(dllexport)
	#endif
#else
	#define EEXPORT
#endif //!EDECLAREEXPORT



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Predefined types of parameters, including config files and shader variables
enum ENBParameterType : long
{
	ENBParam_NONE			=0, //invalid
	ENBParam_FLOAT			=1, //1 float
	ENBParam_INT			=2, //1 int
	ENBParam_HEX			=3, //1 DWORD
	ENBParam_BOOL			=4, //1 BOOL
	ENBParam_COLOR3			=5, //3 float
	ENBParam_COLOR4			=6, //4 float
	ENBParam_VECTOR3		=7, //3 float
	ENBParam_FORCEDWORD		=0x7fffffff  //unused
};

inline long	ENBParameterTypeToSize(ENBParameterType type)
{
	long	size=0;
	if (type==ENBParam_FLOAT) size=4;
	if (type==ENBParam_INT) size=4;
	if (type==ENBParam_HEX) size=4;
	if (type==ENBParam_BOOL) size=4;
	if (type==ENBParam_COLOR3) size=4*3;
	if (type==ENBParam_COLOR4) size=4*4;
	if (type==ENBParam_VECTOR3) size=4*3;
	return size;
}



struct ENBParameter
{
	unsigned char			Data[16];			//array of variables BOOL, INT, FLOAT, max vector of 4 elements
	unsigned long			Size;				//4*4 max
	ENBParameterType		Type;

	ENBParameter()
	{
		for (DWORD k=0; k<16; k++) Data[k]=0;
		Size=0; Type=ENBParam_NONE;
	}
};



enum ENBCallbackType : long
{
	ENBCallback_EndFrame	=1, //called at the end of frame, before displaying result on the screen
	ENBCallback_BeginFrame	=2, //called after frame was displayed, time between end and begin frame may be big enough to execute something heavy in separate thread
	ENBCallback_PreSave		=3, //called before user trying to save config, useful for restoring original parameters
	ENBCallback_PostLoad	=4, //called when parameters are created and loaded, useful for saving original parameters
	//v1001:
	ENBCallback_OnInit		=5, //called when mod is initialized completely, but nothing yet processed, all new resources must be created by plugin
	ENBCallback_OnExit		=6, //called when game or mod are about to close, all new resources must be deleted by plugin
	ENBCallback_PreReset	=7, //called when game or mod destroy internal objects (display mode changes for example), need to destroy all objects in plugin. may not be called ever, but must be handled in some similar way to OnExit
	ENBCallback_PostReset	=8, //called when game or mod re-create internal objects (after display mode changes for example), need to create all objects in plugin again, including pointers to interfaces of d3d. may not be called ever, but must be handled in some similar way to OnInit

	ENBCallback_FORCEDWORD	=0x7fffffff  //unused
};


//v1001:
enum ENBStateType : long
{
	ENBState_IsEditorActive		=1, //is mod editor windows are opened
	ENBState_IsEffectsWndActive	=2, //is shader effects window of mod editor opened
	ENBState_CursorPosX			=3, //position of editor cursor (may not be the same as game cursor)
	ENBState_CursorPosY			=4, //position of editor cursor (may not be the same as game cursor)
	ENBState_MouseLeft			=5, //mouse key state boolean pressed or not
	ENBState_MouseRight			=6, //mouse key state boolean pressed or not
	ENBState_MouseMiddle		=7, //mouse key state boolean pressed or not

	ENBState_FORCEDWORD		=0x7fffffff  //unused
};


//v1001:
struct ENBRenderInfo
{
	//these objects actually pointers to ENBSeries wrapped classes to make sure every change is tracked by mod and restored later
	void					*d3d11device; //ID3D11Device
	void					*d3d11devicecontext; //ID3D11DeviceContext
	void					*dxgiswapchain; //IDXGISwapChain

	DWORD					ScreenSizeX;
	DWORD					ScreenSizeY;

	ENBRenderInfo()
	{
		d3d11device=NULL;
		d3d11devicecontext=NULL;
		dxgiswapchain=NULL;
		ScreenSizeX=0;
		ScreenSizeX=0;
	}
};



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//exported functions from ENBSeries library
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Returns version of SDK used by the ENBSeries, 1000 means 1.0, 1001 means 1.01, etc
//Guaranteed compatibility for all Xxxx versions only, for example 1025 will work with sdk version 1000-1025,
//2025 will work with sdk version 2000-2025, etc. In best cases it's equal to ENBSDKVERSION
typedef long	(*_ENBGetSDKVersion)();
EEXPORT long	ENBGetSDKVersion();


//Returns version of the ENBSeries, 279 means 0.279
typedef long	(*_ENBGetVersion)();
EEXPORT long	ENBGetVersion();


//Returns unique game identifier, for example ENBGAMEID_SKYRIM
typedef long	(*_ENBGetGameIdentifier)();
EEXPORT long	ENBGetGameIdentifier();


//Assign callback function which is executed by ENBSeries at certain moments. This helps to bypass potential bugs
//and increase performance. Argument calltype must be used to select proper action.
//void WINAPI	CallbackFunction(ENBCallbackType calltype); //example function
typedef void	(WINAPI *ENBCallbackFunction)(ENBCallbackType calltype); //declaration of callback function
typedef void	(*_ENBSetCallbackFunction)(ENBCallbackFunction func);
EEXPORT void	ENBSetCallbackFunction(ENBCallbackFunction func);


//Receive value of parameter
//Input "filename" could be NULL to access shader variables instead of configuration files.
//Return FALSE if failed, because function arguments are invalid, parameter not exist or hidden. Also parameters
//may spawn or to be deleted when user modifying shaders, so highly recommended to call it inside callback function.
//For shader variables set filename=NULL
typedef BOOL	(*_ENBGetParameter)(char *filename, char *category, char *keyname, ENBParameter *outparam);
EEXPORT BOOL	ENBGetParameter(char *filename, char *category, char *keyname, ENBParameter *outparam);


//Set value of parameter
//Input "filename" could be NULL to access shader variables instead of configuration files
//Returns FALSE if failed, because function arguments are invalid, parameter not exist, hidden or read only. Also parameters
//may spawn or to be deleted when user modifying shaders.
//Return FALSE if called outside of callback function to protect against graphical artifacts and crashes.
//WARNING! Any values forced by this parameter can be saved by user in editor window, which means corruption of presets,
//so it's highly recommended to warn users about that.
//For shader variables set filename=NULL
typedef BOOL	(*_ENBSetParameter)(char *filename, char *category, char *keyname, ENBParameter *inparam);
EEXPORT BOOL	ENBSetParameter(char *filename, char *category, char *keyname, ENBParameter *inparam);


//v1001:
//Receive various objects for advanced programming
//Return NULL if called too early and not everything initialized yet
typedef ENBRenderInfo*	(*_ENBGetRenderInfo)();
EEXPORT ENBRenderInfo*	ENBGetRenderInfo();


//Receive various states of the mod
//Return boolean or indexed values, depending from which state requested
typedef long	(*_ENBStateType)(ENBStateType state);
EEXPORT long	ENBGetState(ENBStateType state);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



#endif


