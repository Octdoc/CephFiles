#include "application.h"

/*
TODO:
proper zooming
app open settings file
file sorting
*/

#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "windowscodecs.lib")

int Main()
{
	cephimages::Application app;
	app.Init(L"Ceph images", 1280, 720);
	app.Run();
	return 0;
}

int WINAPI wWinMain(HINSTANCE __in hInstance, HINSTANCE __in_opt hPrevInstance, LPWSTR __in szCmdLine, INT __in nCmdShow)
{
	return Main();
}
int wmain()
{
	return Main();
}