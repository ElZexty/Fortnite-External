#include "render.h"

HWND hwnd;

HANDLE(*_CreateThread)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE     lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) = nullptr;
HANDLE __stdcall CreateThread_Esp(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
	return _CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

int main()
{
	MouseController::Init();

	while (hwnd == NULL)
	{
		hwnd = FindWindowA(0, ("Fortnite  "));
		system("cls");
		printf(xor("\n Looking for Fortnite Process!"));
		Sleep(1000);
		printf(xor("\n Process Found, Getting Process ID.."));
	}
	GetWindowThreadProcessId(hwnd, &FNProcID);
	printf("\n\n Process ID Found..");

	KmDrv = new Memory(FNProcID);
	base_address = KmDrv->GetModuleBase("FortniteClient-Win64-Shipping.exe");
	printf(xor("\n Base Address has been Found.."));
	std::cout << "\n Base Address = " << (void*)base_address << std::endl;

	create_overlay();
	directx_init();
	render_loop();

	printf(xor("\n Renderer Loop Initalized | Enjoy Cheating!"));

	return 0;
}