/******************************************************************************
	Copyright (C) 2016-2020 by Streamlabs (General Workings Inc)
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#pragma once

#include <nan.h>
#include <functional>
#include <string>
#include <mutex>
#include <thread>

struct ColorInfo {
	std::string event;
	std::string hex;
	COLORREF color;
};

class ColorPicker : public Nan::AsyncProgressQueueWorker<ColorInfo> {
public:
	ColorPicker(Nan::Callback* cb, Nan::Callback* event, bool showColorWindow, bool showColorHexFlag, bool sendMoveCallbacks);
	~ColorPicker() override;

	void Execute(const AsyncProgressQueueWorker::ExecutionProgress& progress) override;
	void HandleProgressCallback(const ColorInfo* data, size_t size) override;

	static bool IsBusy() { return busy; };
private:
	static bool busy;
	void GetPixelColorOnCursor();
	static std::string GetColorHex(COLORREF& ref);

	bool showColorWindow;
	bool showColorHex;
	bool sendMoveCallbacks;

	std::thread pickingColorThread;
	Nan::Callback* m_event;

	ColorInfo colorPickedInfo;
	HANDLE colorPickedEvent = NULL;
	bool exitWorker = false;
	POINT lastPoint;
	MONITORINFO lastMonitor;

	bool colorWindowLeft = false;
	bool colorWindowTop = false;

	HCURSOR pickerCursor;
	HWND pickerMaskWindow;
	static LRESULT CALLBACK MaskWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK MaksWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void HandleCursorPosition();
	void PickColor();
	void PositionColorWindow();
	void PositionMaskWindow();

	HWND pickerColorWindow;
	static LRESULT CALLBACK ColorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ColorWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void DrawColorWnd();
};