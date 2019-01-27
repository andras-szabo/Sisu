#pragma once

#include <Windows.h>
#include <Windowsx.h>
#include <utility>
#include <string>
#include <iostream>

class IInputService;
class SisuApp;

class WindowManager
{
public:
	static WindowManager* GetInstance();

	WindowManager() = default;
	WindowManager(SisuApp& app, IInputService* const inputService, int width, int height, const std::wstring& caption);

	float AspectRatio() const { return static_cast<float>(_width) / _height; }
	std::pair<int, int> Dimensions() const { return std::pair<int, int>(_width, _height); }

	HWND MainWindowHandle() const { return _hMainWnd; }
	bool IsSetup() const { return _isSetup; }
	void SetText(const std::wstring& text);

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	static WindowManager* _instance;

	void OnWindowActivate(WPARAM wParam);
	void OnWindowResizeMessage(WPARAM wParam, LPARAM lParam);
	void MarkWindowResizeStartOrStop(bool start);

	void Minimize();
	void Maximize();
	void RestoreSize();

	bool _isSetup = false;
	std::wstring _caption;

	HWND _hMainWnd;
	int _width = 0;
	int _height = 0;

	SisuApp& _app;
	IInputService* const _inputService;

	bool _isMinimized = false;
	bool _isMaximized = false;
	bool _isResizing = false;
};