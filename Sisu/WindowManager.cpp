#include "stdafx.h"
#include "WindowManager.h"
#include "Sisu.h"

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return WindowManager::GetInstance()->MsgProc(hwnd, msg, wParam, lParam);
}

WindowManager* WindowManager::_instance = nullptr;
WindowManager* WindowManager::GetInstance()
{
	return _instance;
}

WindowManager::WindowManager(SisuApp& app, int width, int height, const std::wstring& caption) :
	_width(width),
	_height(height),
	_app(app),
	_caption(caption)
{
	_instance = this;

	const wchar_t* WINDOW_CLASS_NAME = L"MainWnd";
	auto appInstanceHandle = app.GetAppInstanceHandle();

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = appInstanceHandle;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		_isSetup = false;
		return;
	}

	RECT R = { 0, 0, _width, _height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int rwidth = R.right - R.left;
	int rheight = R.bottom - R.top;

	_hMainWnd = CreateWindow(WINDOW_CLASS_NAME, caption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rwidth, rheight, 0, 0, appInstanceHandle, 0);

	if (!_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		_isSetup = false;
		return;
	}

	ShowWindow(_hMainWnd, SW_SHOW);
	UpdateWindow(_hMainWnd);

	_isSetup = true;
}

void WindowManager::SetText(const std::wstring& text)
{
	auto fullText = _caption + text;
	SetWindowText(_hMainWnd, fullText.c_str());
}

void WindowManager::OnWindowActivate(WPARAM wParam)
{
	if (LOWORD(wParam) == WA_INACTIVE) { _app.Pause(true); }
	else { _app.Pause(false); }
}

void WindowManager::OnWindowResizeMessage(WPARAM wParam, LPARAM lParam)
{
	_width = LOWORD(lParam);
	_height = HIWORD(lParam);

	if (_app.IsRendererSetup())
	{
		if (wParam == SIZE_MINIMIZED) { Minimize(); }
		else if (wParam == SIZE_MAXIMIZED) { Maximize(); }
		else if (wParam == SIZE_RESTORED) { RestoreSize(); }
	}
}

void WindowManager::Minimize()
{
	_app.Pause(true);
	_isMinimized = true;
	_isMaximized = false;
}

void WindowManager::Maximize()
{
	_app.Pause(false);
	_isMinimized = false;
	_isMaximized = true;

	_app.OnResize();
}

void WindowManager::RestoreSize()
{
	if (_isMinimized)
	{
		_app.Pause(false);
		_isMinimized = false;
		_app.OnResize();
	}
	else if (_isMaximized)
	{
		_app.Pause(false);
		_isMaximized = false;
		_app.OnResize();
	}
	else if (_isResizing)
	{
		// If user is dragging the resize bars, we do not resize 
		// the buffers here because as the user continuously 
		// drags the resize bars, a stream of WM_SIZE messages are
		// sent to the window, and it would be pointless (and slow)
		// to resize for each WM_SIZE message received from dragging
		// the resize bars.  So instead, we reset after the user is 
		// done resizing the window and releases the resize bars, which 
		// sends a WM_EXITSIZEMOVE message.
	}
	else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
	{
		_app.OnResize();
	}
}

void WindowManager::MarkWindowResizeStartOrStop(bool start)
{
	if (start)
	{
		_app.Pause(true);
		_isResizing = true;
	}
	else
	{
		_app.Pause(false);
		_isResizing = false;
		_app.OnResize();
	}
}

LRESULT WindowManager::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE: OnWindowActivate(wParam); return 0;
		case WM_SIZE: OnWindowResizeMessage(wParam, lParam); return 0;
		case WM_ENTERSIZEMOVE: MarkWindowResizeStartOrStop(true); return 0;
		case WM_EXITSIZEMOVE: MarkWindowResizeStartOrStop(false); return 0;
		case WM_DESTROY: PostQuitMessage(0); return 0;

			// WM_MENUCHAR is sent when a menu is active and the user presses
			// a key that does not correspond to any mnemonic or accelerator key
			// What this does is jut make sure it doesn't "beep" when this happens.
		case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE);

			// Catch this message so as to prevent the window from becoming too small
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			_app.OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			_app.OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_MOUSEMOVE:
			_app.OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_KEYDOWN:
			_app.OnKeyDown(wParam);
			return 0;

		case WM_KEYUP:
			_app.OnKeyUp(wParam);
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
