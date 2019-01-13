#include "stdafx.h"
#include "Sisu.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	const int APP_TO_RUN = 2;
	std::wstring appTitle = L"-- app title unset --";

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::unique_ptr<SisuApp> app = std::make_unique<SisuApp>(hInstance);

	try
	{
		if (!app->Init(800, 600, appTitle))
		{
			return 0;
		}

		return app->Run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << "\n";

		//TODO - actually convert that fucking e.what() into wstring

		MessageBox(nullptr, L"Something went wrong. Check logs.", L"HR failed", MB_OK);
		return 0;
	}
}