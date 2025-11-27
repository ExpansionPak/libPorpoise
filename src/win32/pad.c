////////////////////////////////////////////////////////
//
// Coded by Mark A. DeLoura, 3/2/2000
// Copyright (c) 2000 Nintendo of America Inc.
// Questions?  -->  markde01@noa.nintendo.com
//
////////////////////////////////////////////////////////

#include <windows.h>
#include <dinput.h>
#include <dolphin.h>
#include <win32/win32.h>
#include <stdio.h>


////////////////////////////////////////////////////////
// Local types
//

// This is the structure we use to keep track of 
//    DirectInput devices for each joystick found.
typedef struct _DICont {
	LPDIRECTINPUTDEVICE pdev;
	LPDIRECTINPUTDEVICE2 pdev2;
	DWORD				joyType;
} DICont;


////////////////////////////////////////////////////////
// Local global variables
//
static DICont				gControllers[PAD_MAX_CONTROLLERS];
static DWORD				gJoyErrs = 0;
static BOOL					gSystemIsNT;
static DWORD				gJoyNumber = -1;

// For NT systems
static UINT gJoyIDlist[] = {JOYSTICKID1,JOYSTICKID2};
static UINT	gNumJoyDevs;


////////////////////////////////////////////////////////
// External global variables
//

// These are used to connect with Windows
extern HWND					global_hWnd;
extern HINSTANCE			global_hInstance;
extern WinKey				winKey;


// _PADInitJoystickInput()
//
// Callback routine which is called for each available joystick by EnumDevices().
// We're assuming there is just one stick.
//
BOOL FAR PASCAL _PADInitJoystickInput(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef) { 
	LPDIRECTINPUTDEVICE pdev;
	LPDIRECTINPUTDEVICE2 pdev2;
	LPDIRECTINPUT lpdi = (LPDIRECTINPUT) pvRef;
	HRESULT	hres;
	DIPROPRANGE diprg;
	DIDEVCAPS devCaps;
	DWORD	joyType;

	// Inc joynumber to keep track of multiple joysticks
	gJoyNumber++;

	// create the DirectInput joystick device (pdev and pdev2)
	if (lpdi->lpVtbl->CreateDevice(lpdi, &(pdinst->guidInstance), 
		&(gControllers[gJoyNumber].pdev), NULL) != DI_OK) { 
		OutputDebugString("IDirectInput::CreateDevice FAILED\n"); 
		return DIENUM_CONTINUE; 
	} else {
		pdev = gControllers[gJoyNumber].pdev;
	    hres = pdev->lpVtbl->QueryInterface(pdev, &IID_IDirectInputDevice2, 
			(LPVOID *)&(gControllers[gJoyNumber].pdev2));
		pdev2 = gControllers[gJoyNumber].pdev2;
		// pdev->lpVtbl->Release(pdev); 
	}

	// Get capabilities of this stick (um, why?)
	devCaps.dwSize = sizeof(DIDEVCAPS);
	if (pdev2->lpVtbl->GetCapabilities(pdev2, (LPDIDEVCAPS)&devCaps) != DI_OK) {
		OutputDebugString("IDirectInputDevice::GetCapabilities FAILED\n"); 
		pdev2->lpVtbl->Release(pdev2); 
		return DIENUM_CONTINUE; 
	}

	// Determine what joystick we have.
	if (!strcmp(pdinst->tszInstanceName, "Microsoft SideWinder game pad")) {
		gControllers[gJoyNumber].joyType = PCPAD_TYPE_GAMEPAD;
	} else if (!strcmp(pdinst->tszInstanceName, "Microsoft SideWinder Game Pad Pro")) {
		gControllers[gJoyNumber].joyType = PCPAD_TYPE_GAMEPADPRO;
	} else if (!strcmp(pdinst->tszInstanceName, "Microsoft SideWinder Freestyle Pro")) {
		gControllers[gJoyNumber].joyType = PCPAD_TYPE_FREESTYLEPRO;
	} else if (!strcmp(pdinst->tszInstanceName, "Microsoft SideWinder 3D Pro")) {
		gControllers[gJoyNumber].joyType = PCPAD_TYPE_3DPRO;
	} else if (!strcmp(pdinst->tszInstanceName, "Macally iShock")) {
		gControllers[gJoyNumber].joyType = PCPAD_TYPE_ISHOCK;
	} else if (!strcmp(pdinst->tszInstanceName, "Gravis GamePad Pro")) {
		gControllers[gJoyNumber].joyType = PCPAD_TYPE_GRAVISPADPRO;
	} else {
		if (devCaps.dwButtons == 20) {
			gControllers[gJoyNumber].joyType = PCPAD_TYPE_ISHOCK;
		} else {
			gControllers[gJoyNumber].joyType = PCPAD_TYPE_UNKNOWN;
		}
	}
	joyType = gControllers[gJoyNumber].joyType;

	// Set joystick data format (c_dfDIJoystick2 is the super joystick status format)
	if (pdev2->lpVtbl->SetDataFormat(pdev2, &c_dfDIJoystick2) != DI_OK) { 
		OutputDebugString("IDirectInputDevice::SetDataFormat FAILED\n"); 
		pdev2->lpVtbl->Release(pdev2); 
		return DIENUM_CONTINUE; 
	}

	// Set joystick's cooperative level
	if((hres=pdev2->lpVtbl->SetCooperativeLevel(pdev2, global_hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND)) != DI_OK) { 
		OutputDebugString("IDirectInputDevice::SetCooperativeLevel FAILED\n"); 
		pdev2->lpVtbl->Release(pdev2); 
		return DIENUM_CONTINUE; 
	}

	// Acquire the joystick 
	if ((hres = pdev2->lpVtbl->Acquire(pdev2)) != DI_OK) {
		OutputDebugString("IDirectInputDevice::Acquire FAILED\n"); 
		pdev2->lpVtbl->Release(pdev2); 
		return DIENUM_CONTINUE; 
	}

	// Set X-Axis Range
	diprg.diph.dwSize       = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj        = DIJOFS_X; 
	diprg.diph.dwHow        = DIPH_BYOFFSET; 
	diprg.lMin              = -127; 
	diprg.lMax              = +127; 
	if FAILED(pdev2->lpVtbl->SetProperty(pdev2, DIPROP_RANGE, &diprg.diph)) { 
		OutputDebugString("IDirectInputDevice::SetProperty(DIPROP_RANGE X) FAILED\n"); 
		pdev2->lpVtbl->Release(pdev2); 
		return FALSE; 
	}

	// Set Y-Axis Range
	diprg.diph.dwSize       = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj        = DIJOFS_Y; 
	diprg.diph.dwHow        = DIPH_BYOFFSET; 
	diprg.lMin              = -127; 
	diprg.lMax              = +127; 
	if FAILED(pdev2->lpVtbl->SetProperty(pdev2, DIPROP_RANGE, &diprg.diph)) { 
		OutputDebugString("IDirectInputDevice::SetProperty(DIPROP_RANGE Y) FAILED\n"); 
		pdev2->lpVtbl->Release(pdev2); 
		return FALSE; 
	}

	// Set right joystick X-Axis Range
	if ((joyType == PCPAD_TYPE_ISHOCK) || (joyType == PCPAD_TYPE_FREESTYLEPRO) ||
		(joyType == PCPAD_TYPE_3DPRO)) {
		diprg.diph.dwSize       = sizeof(diprg); 
		diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
		diprg.diph.dwObj        = DIJOFS_Z; 
		diprg.diph.dwHow        = DIPH_BYOFFSET; 
		diprg.lMin              = -127; 
		diprg.lMax              = +127; 
		if FAILED(pdev2->lpVtbl->SetProperty(pdev2, DIPROP_RANGE, &diprg.diph)) { 
			OutputDebugString("IDirectInputDevice::SetProperty(DIPROP_RANGE Z) FAILED\n"); 
			//pdev2->Release(); 
			return FALSE; 
		}
	}

	// Set right joystick Y-Axis Range
	if ((joyType == PCPAD_TYPE_ISHOCK) || (joyType == PCPAD_TYPE_3DPRO)) {
		diprg.diph.dwSize       = sizeof(diprg); 
		diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
		diprg.diph.dwObj        = DIJOFS_RZ; 
		diprg.diph.dwHow        = DIPH_BYOFFSET; 
		diprg.lMin              = -127; 
		diprg.lMax              = +127; 
		if FAILED(pdev2->lpVtbl->SetProperty(pdev2, DIPROP_RANGE, &diprg.diph)) { 
			OutputDebugString("IDirectInputDevice::SetProperty(DIPROP_RANGE RZ) FAILED\n"); 
			//pdev2->Release(); 
			return FALSE; 
		}
	}

	return TRUE;
}


// _PADReacquireInput()
//
// Called if for some reason we've lost access to the joystick device.
//
BOOL _PADReacquireInput(u32 jnum) { 
    HRESULT hRes; 
 
    // if we have a current device 
    if (gControllers[jnum].pdev2) { 
       // acquire the device 
		hRes = (gControllers[jnum].pdev2)->lpVtbl->Acquire(gControllers[jnum].pdev2); 
       // The call above is a macro that expands to: 
       // g_pdevCurrent->lpVtbl->Acquire(g_pdevCurrent); 
 
       if (SUCCEEDED(hRes)) { 
          // acquisition successful 
          return TRUE; 
       } else { 
          // acquisition failed 
          return FALSE; 
       } 
    } else { 
       // we don't have a current device 
       return FALSE; 
    } 
} 


// PADInit()
//
// Call to setup access to the joystick.
// NT does not have DirectX, all other platforms do.
//   Determine if we're on NT in this routine.
//
BOOL PADInit(void) {
	HRESULT			hr; 
	LPDIRECTINPUT	lpdi; 
	PADStatus		status[4]; 	
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Determine if we're running on NT
	//
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) ) {
	  // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )  {
			OutputDebugString("Could not determine system version\n");
			return FALSE;
		}
	}

	if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion <= 4)) {
		gSystemIsNT = TRUE;
	} else {
		gSystemIsNT = FALSE;
	}


	if (gSystemIsNT) {
		gNumJoyDevs = joyGetNumDevs();
		if (gNumJoyDevs == 0) {
			// No joystick is available!
			OutputDebugString("Could not initialize joystick\n");
			return FALSE;
		}

	} else { // system is not NT, safe to use DirectX

		// Create DirectInput object
		//
		hr = DirectInputCreate(global_hInstance, DIRECTINPUT_VERSION, &lpdi, NULL); 
		if FAILED(hr) { 
			// DirectInput not available; take appropriate action 
			OutputDebugString("Could not create DirectInput instance\n");
			return FALSE;
		} 

		// Enumerate currently-attached joysticks through InitJoystickInput() callback
		lpdi->lpVtbl->EnumDevices(lpdi, DIDEVTYPE_JOYSTICK, _PADInitJoystickInput, lpdi, DIEDFL_ATTACHEDONLY); 
	}

	// Test joystick read
	PADRead(status);

	return TRUE;
}


// _PADConvertStickStatus()
//
// Do something different depending on the type of gamepad we found.
//   If the gamepad is unknown, guess that the Microsoft Gamepad is
//   a close approximation.
void _PADConvertStickStatus(PADStatus* status, u32 jnum, DIJOYSTATE2* joyState) {
	DWORD		joyType;
	u16			joyButtons;

	// Examine the current stick type
	joyType = gControllers[jnum].joyType;

	// These are common among all sticks
	status[jnum].stickX = joyState->lX;
	status[jnum].stickY = -joyState->lY;
	joyButtons = 0;

	if ((joyType == PCPAD_TYPE_GAMEPAD) || (joyType == PCPAD_TYPE_UNKNOWN)) {
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_A]) {
			joyButtons |= PAD_BUTTON_A;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_X]) {
			joyButtons |= PAD_BUTTON_B;
		} 
#if 0
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_B]) {
			joyButtons |= PAD_BUTTON_C_DOWN;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_C]) {
			joyButtons |= PAD_BUTTON_C_RIGHT;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_Y]) {
			joyButtons |= PAD_BUTTON_C_LEFT;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_Z]) {
			joyButtons |= PAD_BUTTON_C_UP;
		} 

		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_L]) {
			joyButtons |= PAD_TRIGGER_L;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_R]) {
			joyButtons |= PAD_TRIGGER_R;
		} 

		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_SELECT]) {
			joyButtons |= PAD_TRIGGER_Z;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPAD_BUTTON_START]) {
			joyButtons |= PAD_BUTTON_START;
		} 
#endif

	} else if (joyType == PCPAD_TYPE_GAMEPADPRO) {
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_A]) {
			joyButtons |= PAD_BUTTON_A;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_X]) {
			joyButtons |= PAD_BUTTON_B;
		} 
#if 0
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_B]) {
			joyButtons |= PAD_BUTTON_C_DOWN;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_C]) {
			joyButtons |= PAD_BUTTON_C_RIGHT;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_Y]) {
			joyButtons |= PAD_BUTTON_C_LEFT;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_Z]) {
			joyButtons |= PAD_BUTTON_C_UP;
		} 

		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_L]) {
			joyButtons |= PAD_TRIGGER_L;
		} 
		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_R]) {
			joyButtons |= PAD_TRIGGER_R;
		} 

#endif

		if (joyState->rgbButtons[PCPAD_GAMEPADPRO_BUTTON_START]) {
			joyButtons |= PAD_BUTTON_MENU;
		} 

	} else if (joyType == PCPAD_TYPE_FREESTYLEPRO) {
		// The tilt sensor comes in as stickX/Y
		// This is the dial
		status[jnum].substickY = -joyState->lZ;

		// DPad is on the POV control, overrides tilt sensor (in case sensor is off)
		switch (joyState->rgdwPOV[0]) {
		case PCPAD_FREESTYLEPRO_UP:
			status[jnum].stickY = 127;
			break;
		case PCPAD_FREESTYLEPRO_DOWN:
			status[jnum].stickY = -127;
			break;
		case PCPAD_FREESTYLEPRO_LEFT:
			status[jnum].stickX = -127;
			break;
		case PCPAD_FREESTYLEPRO_RIGHT:
			status[jnum].stickX = 127;
			break;
		case PCPAD_FREESTYLEPRO_UPLEFT:
			status[jnum].stickX = -127;
			status[jnum].stickY = 127;
			break;
		case PCPAD_FREESTYLEPRO_UPRIGHT:
			status[jnum].stickX = 127;
			status[jnum].stickY = 127;
			break;
		case PCPAD_FREESTYLEPRO_DOWNLEFT:
			status[jnum].stickX = -127;
			status[jnum].stickY = -127;
			break;
		case PCPAD_FREESTYLEPRO_DOWNRIGHT:
			status[jnum].stickX = 127;
			status[jnum].stickY = -127;
			break;
		}
		
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_A]) {
			joyButtons |= PAD_BUTTON_A;
		} 
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_X]) {
			joyButtons |= PAD_BUTTON_B;
		} 
#if 0
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_B]) {
			joyButtons |= PAD_BUTTON_C_DOWN;
		} 
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_C]) {
			joyButtons |= PAD_BUTTON_C_RIGHT;
		} 
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_Y]) {
			joyButtons |= PAD_BUTTON_C_LEFT;
		} 
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_Z]) {
			joyButtons |= PAD_BUTTON_C_UP;
		} 

		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_L]) {
			joyButtons |= PAD_TRIGGER_L;
		} 
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_R]) {
			joyButtons |= PAD_TRIGGER_R;
		} 

		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_SELECT]) {
			joyButtons |= PAD_TRIGGER_Z;
		} 
		if (joyState->rgbButtons[PCPAD_FREESTYLEPRO_BUTTON_START]) {
			joyButtons |= PAD_BUTTON_START;
		} 
#endif

	} else if (joyType == PCPAD_TYPE_3DPRO) {
		// The joystick comes in as stickX/Y
		// This is the throttle
		status[jnum].substickY = -joyState->lZ;
		// This is the joystick rotation
		status[jnum].substickX = joyState->lRz;

		// DPad is on the POV control
		switch (joyState->rgdwPOV[0]) {
		case PCPAD_3DPRO_UP:
			status[jnum].stickY = 127;
			break;
		case PCPAD_3DPRO_DOWN:
			status[jnum].stickY = -127;
			break;
		case PCPAD_3DPRO_LEFT:
			status[jnum].stickX = -127;
			break;
		case PCPAD_3DPRO_RIGHT:
			status[jnum].stickX = 127;
			break;
		case PCPAD_3DPRO_UPLEFT:
			status[jnum].stickX = -127;
			status[jnum].stickY = 127;
			break;
		case PCPAD_3DPRO_UPRIGHT:
			status[jnum].stickX = 127;
			status[jnum].stickY = 127;
			break;
		case PCPAD_3DPRO_DOWNLEFT:
			status[jnum].stickX = -127;
			status[jnum].stickY = -127;
			break;
		case PCPAD_3DPRO_DOWNRIGHT:
			status[jnum].stickX = 127;
			status[jnum].stickY = -127;
			break;
		}

		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_STICKBOTTOM]) {
			joyButtons |= PAD_BUTTON_A;
		} 
		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_STICKTOP]) {
			joyButtons |= PAD_BUTTON_B;
		} 
#if 0
		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_BASELL]) {
			joyButtons |= PAD_BUTTON_C_DOWN;
		} 
		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_BASELR]) {
			joyButtons |= PAD_BUTTON_C_RIGHT;
		} 
		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_BASEUL]) {
			joyButtons |= PAD_BUTTON_C_LEFT;
		} 
		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_BASEUR]) {
			joyButtons |= PAD_BUTTON_C_UP;
		} 

		if (joyState->rgbButtons[PCPAD_3DPRO_TRIGGER]) {
			joyButtons |= PAD_TRIGGER_Z;
		} 
		if (joyState->rgbButtons[PCPAD_3DPRO_BUTTON_THUMB]) {
			joyButtons |= PAD_BUTTON_START;
		} 
#endif

	} else if (joyType == PCPAD_TYPE_ISHOCK) {

		// The left analog stick comes in as shiftX/Y
		// This is the right analog stick
		status[jnum].substickX = -joyState->lRz;
		status[jnum].substickY = -joyState->lZ;

		// Buttons
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_X]) {
			joyButtons |= PAD_BUTTON_A;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_CIRCLE]) {
			joyButtons |= PAD_BUTTON_B;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_SQUARE]) {
			joyButtons |= PAD_BUTTON_X;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_TRIANGLE]) {
			joyButtons |= PAD_BUTTON_Y;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_L1]) {
			joyButtons |= PAD_TRIGGER_L;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_R1]) {
			joyButtons |= PAD_TRIGGER_R;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_R2]) {
			joyButtons |= PAD_TRIGGER_Z;
		} 
#if 0
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_L2]) {
			joyButtons |= PAD_TRIGGER_L;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_SELECT]) {
			joyButtons |= PAD_TRIGGER_R;
		} 
#endif
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_START]) {
			joyButtons |= PAD_BUTTON_MENU;
		} 

		// Override left analog controls with the d-pad
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_DPAD_DOWN]) {
			joyButtons |= PAD_BUTTON_DOWN;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_DPAD_UP]) {
			joyButtons |= PAD_BUTTON_UP;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_DPAD_LEFT]) {
			joyButtons |= PAD_BUTTON_LEFT;
		} 
		if (joyState->rgbButtons[PCPAD_ISHOCK_BUTTON_DPAD_RIGHT]) {
			joyButtons |= PAD_BUTTON_RIGHT;
		} 
	} else if (joyType == PCPAD_TYPE_GRAVISPADPRO) { 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_YELLOW]) {
			joyButtons |= PAD_BUTTON_A;
		} 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_RED]) {
			joyButtons |= PAD_BUTTON_B;
		} 
#if 0
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_GREEN]) {
			joyButtons |= PAD_BUTTON_C_DOWN;
		} 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_R1]) {
			joyButtons |= PAD_BUTTON_C_RIGHT;
		} 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_L1]) {
			joyButtons |= PAD_BUTTON_C_LEFT;
		} 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_BLUE]) {
			joyButtons |= PAD_BUTTON_C_UP;
		} 

		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_L2]) {
			joyButtons |= PAD_TRIGGER_L;
		} 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_R2]) {
			joyButtons |= PAD_TRIGGER_R;
		} 

		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_SELECT]) {
			joyButtons |= PAD_TRIGGER_Z;
		} 
		if (joyState->rgbButtons[PCPAD_GRAVISPADPRO_BUTTON_START]) {
			joyButtons |= PAD_BUTTON_START;
		} 
#endif
	}

	status[jnum].button = joyButtons;
}


// PADRead()
//
// Get latest controller values.
// Currently, works properly with MS Sidewinder tilt stick.
// Other joysticks: your mileage may vary.
//
void PADRead (PADStatus* status) {
	static u16 error = 0;
	HRESULT		hRes; 
	u16			joyButtons;
	DIJOYSTATE2	joyState;
	u32 jnum;

	// If the system is an NT machine, try to do something intelligent.
	//   If the joystick is a Sidewinder3D, we're in luck since that is directly
	//   supported by a driver.  If it's not that joystick, who knows what we'll
	//   end up with for output here...
	// If the system is not an NT machine, we have a lot of different options!
	if (gSystemIsNT) {
		JOYINFOEX ji;
		MMRESULT result;
		s32 xpos, ypos, zpos, rpos;

		if (error || gNumJoyDevs == 0) {
			status[0].stickX = winKey.stickX;
			status[0].stickY = winKey.stickY;
			status[0].substickX = winKey.substickX;
			status[0].substickY = winKey.substickY;
			status[0].triggerLeft = winKey.triggerLeft;
			status[0].triggerRight = winKey.triggerRight;
			status[0].button = winKey.button;
			return;
		}

		for (jnum=0; jnum<gNumJoyDevs; jnum++) {
			ZeroMemory(&ji,sizeof(ji));
			ji.dwSize=sizeof(JOYINFOEX);
			ji.dwFlags=JOY_RETURNBUTTONS|JOY_RETURNX|JOY_RETURNY|JOY_RETURNPOV|JOY_RETURNR|JOY_RETURNZ|JOY_USEDEADZONE;

			// Read and process input
			result=joyGetPosEx(gJoyIDlist[jnum],&ji);

			switch(result) {
			case JOYERR_UNPLUGGED:
			case MMSYSERR_BADDEVICEID:
			case MMSYSERR_INVALPARAM:
			case MMSYSERR_NODRIVER:
				error = 1;
			break;
			default:

				xpos=ji.dwXpos-32767;
				ypos=ji.dwYpos-32767;
				zpos=ji.dwRpos-32767;
				rpos=ji.dwZpos-32767;

				xpos*=127.0f/32767.0f;
				ypos*=-127.0f/32767.0f;
				zpos*=127.0f/32767.0f;
				rpos*=-127.0f/32767.0f;

				// Buttons
				joyButtons = 0;
				if (ji.dwButtons&0x01) joyButtons|=PAD_BUTTON_X;
				if (ji.dwButtons&0x02) joyButtons|=PAD_BUTTON_MENU;
				//if (ji.dwButtons&0x02) joyButtons|=PAD_BUTTON_START;
				if (ji.dwButtons&0x04) joyButtons|=PAD_BUTTON_B;
				if (ji.dwButtons&0x08) joyButtons|=PAD_BUTTON_A;
#if 0
				switch (ji.dwPOV) {
				case JOY_POVBACKWARD:
					joyButtons |= PAD_BUTTON_C_DOWN;
					break;
				case JOY_POVFORWARD:
					joyButtons |= PAD_BUTTON_C_UP;
					break;
				case JOY_POVLEFT:
					joyButtons |= PAD_BUTTON_C_LEFT;
					break;
				case JOY_POVRIGHT:
					joyButtons |= PAD_BUTTON_C_RIGHT;
					break;
				default:
					break;
				}
#endif
				status[jnum].stickX=xpos;
				status[jnum].stickY=ypos;
				status[jnum].substickX=zpos;
				status[jnum].substickY=rpos;
				status[jnum].button=joyButtons;
				break;
			}
		}

	} else { // System is not NT

		if (!gControllers[0].pdev2) {
			if (!error) {
				OutputDebugString("****************************\nUnable to initialize Controller!\nDefaulting to Keyboard Controls\nUse Number Pad & Arrows.\n****************************\n");
				error = 1;
			}
			status[0].stickX = winKey.stickX;
			status[0].stickY = winKey.stickY;
			status[0].substickX = winKey.substickX;
			status[0].substickY = winKey.substickY;
			status[0].triggerLeft = winKey.triggerLeft;
			status[0].triggerRight = winKey.triggerRight;
			status[0].button = winKey.button;
			return;
		}

		// Loop through the available sticks
		for (jnum=0; jnum<=gJoyNumber; jnum++) {

			// poll the joystick to read the current state
			// if we receive an error, print an out a debug message and attempt to 
			//   reacquire input, if applicable.
			hRes = (gControllers[jnum].pdev2)->lpVtbl->Poll(gControllers[jnum].pdev2);
			if (hRes != DI_OK) { 
#ifdef PRINT_ERRORS
				// did the read fail because we lost input for some reason? 
				// if so, then attempt to reacquire. If the second acquire 
				// fails, then the error from GetDeviceData will be 
				// DIERR_NOTACQUIRED, so we won't get stuck an infinite loop. 
				if (hRes == DIERR_INPUTLOST)
					OutputDebugString("Input lost\n");
				if (hRes == DIERR_INVALIDPARAM)
					OutputDebugString("invalid param\n");
				if (hRes == DIERR_NOTACQUIRED)
					OutputDebugString("not acquired\n");
				if (hRes == DIERR_NOTINITIALIZED)
					OutputDebugString("Not initialized\n");
				if (hRes == E_PENDING)
					OutputDebugString("E_PENDING\n");
#endif

				if((hRes == DIERR_INPUTLOST) || (hRes == DIERR_NOTACQUIRED))
					_PADReacquireInput(jnum); 
 
				// return the fact that we did not read any data 
				// one or two errors is normal (as normal as that is...)
				gJoyErrs++;
				if (gJoyErrs >= 5)
					OutputDebugString("Error getting controller\n");
			}  else {
				// get data from the joystick 
				hRes = (gControllers[jnum].pdev2)->lpVtbl->GetDeviceState(gControllers[jnum].pdev2, sizeof(DIJOYSTATE2), &joyState); 
			}

			_PADConvertStickStatus(status, jnum, &joyState);
		}
	}
}


BOOL PADReset(u32 mask)
{
	return TRUE;
}