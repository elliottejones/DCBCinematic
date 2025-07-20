#include "pch.h"
#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include <cmath> // for sqrt
#include "MinHook.h"

// ################## PARAMETERS THAT YOU CAN TWEAK ##################

static double friction = 0.9; 
static double sensitivity = 0.05;  
static double maxVelocity = 15.0;  

// ####################################################################

static double velocityX = 0.0;
static double velocityY = 0.0;
static double accumX = 0.0;
static double accumY = 0.0;
static bool   smoothingEnabled = false;

static SHORT prevF6State = 0;

using GetRawInputData_t = UINT(WINAPI*)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
static GetRawInputData_t TrueGetRawInputData = nullptr;

UINT WINAPI HookedGetRawInputData(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader)
{
    UINT ret = TrueGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

    SHORT f6 = GetAsyncKeyState(VK_F6);
    if ((f6 & 0x8000) && !(prevF6State & 0x8000)) {
        smoothingEnabled = !smoothingEnabled;
        velocityX = velocityY = accumX = accumY = 0.0;
    }
    prevF6State = f6;

    if (smoothingEnabled && uiCommand == RID_INPUT && pData && *pcbSize >= sizeof(RAWINPUT)) {
        RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(pData);
        if (raw->header.dwType == RIM_TYPEMOUSE) {
            LONG dx = raw->data.mouse.lLastX;
            LONG dy = raw->data.mouse.lLastY;

            velocityX += dx * sensitivity;
            velocityY += dy * sensitivity;

            double speed = std::sqrt(velocityX * velocityX + velocityY * velocityY);
            if (speed > maxVelocity) {
                velocityX = (velocityX / speed) * maxVelocity;
                velocityY = (velocityY / speed) * maxVelocity;
            }
        }
    }
    return ret;
}

DWORD WINAPI HeartbeatThread(LPVOID)
{
    const DWORD frameMs = 16; // ~60hz update
    while (true) {
        Sleep(frameMs);
        if (!smoothingEnabled)
            continue;

        velocityX *= friction;
        velocityY *= friction;

        accumX += velocityX;
        accumY += velocityY;
        LONG moveX = static_cast<LONG>(accumX);
        LONG moveY = static_cast<LONG>(accumY);
        accumX -= moveX;
        accumY -= moveY;

        // inject movement
        if (moveX != 0 || moveY != 0) {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            input.mi.dx = moveX;
            input.mi.dy = moveY;
            SendInput(1, &input, sizeof(input));
        }
    }
}

// starts heartbeat update loop and installs the hook
void InstallHook()
{
    if (MH_Initialize() != MH_OK)
        return;

    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (!hUser32)
        return;

    FARPROC target = GetProcAddress(hUser32, "GetRawInputData");
    if (!target)
        return;

    if (MH_CreateHook(target, &HookedGetRawInputData,
        reinterpret_cast<LPVOID*>(&TrueGetRawInputData)) != MH_OK)
        return;

    if (MH_EnableHook(target) != MH_OK)
        return;

    // frame based heartbeat thread
    CreateThread(NULL, 0, HeartbeatThread, NULL, 0, NULL);
}

// entry point
DWORD WINAPI MainThread(LPVOID)
{
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    InstallHook();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    return TRUE;
}
