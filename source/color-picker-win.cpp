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

#include "color-picker-win.h"
#include "util-win.h"

#include <sstream>
#include <ios>
#include <iomanip>
#include <iostream>

#define MOUSECLICK_EVENT "mouseClick"
#define MOUSEMOVE_EVENT "mouseMove"
#define FINISHED_EVENT "nonEvent"

HINSTANCE hInst = NULL;
const DWORD IDT_TIMER_OVER_WINDOW = 1001;
const DWORD ID_PICKER_CURSOR = 1010;
bool ColorPicker::busy = false;

int ColorWindowWidth = 75;
int ColorWindowHeight = 45;

using namespace Nan;
using namespace v8;

ColorPicker::ColorPicker(Nan::Callback* cb, Nan::Callback* event, bool showColorWindowFlag, bool sendMoveCallbacksFlag) :
	AsyncProgressQueueWorker(cb),
	pickingColorThread(),
	m_event(event),
    showColorWindow(showColorWindowFlag),
    sendMoveCallbacks(sendMoveCallbacksFlag)
{
	busy = true;
	colorPickedEvent = CreateEvent(nullptr, false, false, L"");
}

ColorPicker::~ColorPicker()
{
	CloseHandle(colorPickedEvent);
	delete m_event;
	busy = false;
}

void ColorPicker::Execute(const AsyncProgressQueueWorker::ExecutionProgress& progress)
{
	pickingColorThread = std::thread(&ColorPicker::GetPixelColorOnCursor, this);

	while (!exitWorker) {
		WaitForSingleObject(colorPickedEvent, INFINITE);

		if (colorPickedInfo.event.compare(FINISHED_EVENT) != 0) {
			progress.Send(&colorPickedInfo, 1);
		}
	}

	if (pickingColorThread.joinable()) {
		pickingColorThread.join();
	}
}

void ColorPicker::HandleProgressCallback(const ColorInfo* data, size_t size)
{
	Nan::HandleScope scope;

	v8::Local<v8::Object> eventInfo = Nan::New<v8::Object>();
	Nan::Set(eventInfo, Nan::New("event").ToLocalChecked(), New<v8::String>(data->event.c_str()).ToLocalChecked());
	Nan::Set(eventInfo, Nan::New("hex").ToLocalChecked(), New<v8::String>(data->hex.c_str()).ToLocalChecked());
	v8::Local<v8::Value> argv[] = { eventInfo };

	AsyncResource resource("color-picker:Event");
	m_event->Call(1, argv, &resource);
}

void ColorPicker::GetPixelColorOnCursor()
{
	hInst = GetModuleHandle(NULL);

	pickerCursor = LoadCursor(GetCurrentModule(), MAKEINTRESOURCE(ID_PICKER_CURSOR));

	WNDCLASSEX wcm = { 0 };
	wcm.cbSize = sizeof(wcm);
	wcm.style = 0;
	wcm.lpfnWndProc = ColorPicker::MaskWndProc;
	wcm.cbClsExtra = 0;
	wcm.cbWndExtra = 0;
	wcm.hInstance = hInst;
	wcm.hCursor = pickerCursor;
	wcm.lpszClassName = L"picker_mask_win_cls";
	wcm.hbrBackground = NULL;
	UnregisterClass(L"picker_mask_win_cls", hInst);

	WNDCLASSEX wcc = { 0 };
	wcc.cbSize = sizeof(wcm);
	wcc.style = CS_HREDRAW | CS_VREDRAW;
	wcc.lpfnWndProc = ColorPicker::ColorWndProc;
	wcc.cbClsExtra = 0;
	wcc.cbWndExtra = 0;
	wcc.hInstance = hInst;
	wcc.hCursor = pickerCursor;
	wcc.lpszClassName = L"picker_color_win_cls";
	wcc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	UnregisterClass(L"picker_color_win_cls", hInst);

	if (RegisterClassEx(&wcm)) {
		pickerMaskWindow = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, L"picker_mask_win_cls", L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInst, this);
		if (pickerMaskWindow) {
			UpdateWindow(pickerMaskWindow);
			ShowWindow(pickerMaskWindow, SW_SHOW);

			SetTimer(pickerMaskWindow, IDT_TIMER_OVER_WINDOW, 100, (TIMERPROC)NULL);

			RegisterClassEx(&wcc);
			pickerColorWindow = CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_TOPMOST, L"picker_color_win_cls", L"", WS_POPUP | WS_BORDER, 0, 0, ColorWindowWidth, ColorWindowHeight, NULL, NULL, hInst, this);
			UpdateWindow(pickerColorWindow);

			MSG msg;
			while (GetMessage(&msg, 0, 0, 0) > 0) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			KillTimer(pickerMaskWindow, IDT_TIMER_OVER_WINDOW);
			DestroyWindow(pickerMaskWindow);
			DestroyWindow(pickerColorWindow);
		}
		UnregisterClass(L"picker_mask_win_cls", hInst);
	}

	DestroyCursor(pickerCursor);

	colorPickedInfo.event = FINISHED_EVENT;
	exitWorker = true;
	SetEvent(colorPickedEvent);
}

LRESULT CALLBACK ColorPicker::MaskWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCREATE:
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		ColorPicker* colorPicker = (ColorPicker*)(cs->lpCreateParams);
		colorPicker->pickerMaskWindow = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)colorPicker);
		return colorPicker->MaksWndHandler(hWnd, message, wParam, lParam);
	}
	default:
	{
		ColorPicker* colorPicker = reinterpret_cast<ColorPicker*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!colorPicker)
			return FALSE;
		return colorPicker->MaksWndHandler(hWnd, message, wParam, lParam);
	}
	}
}

LRESULT CALLBACK ColorPicker::MaksWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
    case WM_MOUSEMOVE:
        GetCursorPos(&lastPoint);
        PositionColorWindow();
        break;
	case WM_SETCURSOR:
		SetCursor(pickerCursor);
		break;
	case WM_TIMER:
		if (wParam == IDT_TIMER_OVER_WINDOW)
		{
			HandleCursorPosition();
		}
		break;
	case WM_LBUTTONUP:
		colorPickedInfo.event = MOUSECLICK_EVENT;
		SetEvent(colorPickedEvent);
		DestroyWindow(pickerMaskWindow);
		break;
	case WM_RBUTTONUP:
		colorPickedInfo.event = MOUSECLICK_EVENT;
		SetEvent(colorPickedEvent);
		DestroyWindow(pickerMaskWindow);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(pickerMaskWindow);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ColorPicker::HandleCursorPosition()
{
	GetCursorPos(&lastPoint);

	PickColor();

	PositionMaskWindow();
}

void ColorPicker::PickColor()
{
	HDC screenDC;
	screenDC = GetDC(nullptr);
	if (screenDC != nullptr) {
		COLORREF colorRef = CLR_INVALID;
		colorRef = GetPixel(screenDC, lastPoint.x, lastPoint.y);
		ReleaseDC(GetDesktopWindow(), screenDC);

		if (colorRef != CLR_INVALID) {
			ColorInfo colorInfo;
			colorInfo.hex = GetColorHex(colorRef);
			colorInfo.color = colorRef;
			colorInfo.event = MOUSEMOVE_EVENT;
			colorPickedInfo = colorInfo;
            
            if(sendMoveCallbacks)
			    SetEvent(colorPickedEvent);
            
            RedrawWindow(pickerColorWindow, 0, 0, RDW_INVALIDATE);
		}
	}
}

void ColorPicker::PositionColorWindow()
{

	HDC winDC = GetDC(pickerColorWindow);
	if (winDC)
	{
		RECT rc = { 0 };
		rc.right = 1000;
		rc.bottom = 1000;
		DrawText(winDC, L"0xFFFFFFFF", -1, &rc, DT_CALCRECT);
		ColorWindowWidth = rc.right + 10;
		ColorWindowHeight = rc.bottom + 27;

		ReleaseDC(pickerColorWindow, winDC);
	}
    
    if(showColorWindow)
    {
	    SetWindowPos(pickerColorWindow, 0, lastPoint.x + 5, lastPoint.y + 5, ColorWindowWidth, ColorWindowHeight, SWP_SHOWWINDOW);
	    RedrawWindow(pickerColorWindow, 0, 0, RDW_INVALIDATE);
    }
}

void ColorPicker::PositionMaskWindow()
{
	HWND under_coursor = WindowFromPoint(lastPoint);
	if (under_coursor != pickerMaskWindow) {
		HMONITOR monitor = MonitorFromPoint(lastPoint, MONITOR_DEFAULTTONULL);
		if (monitor) {
			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			::GetMonitorInfo(monitor, (LPMONITORINFO)&mi);
			int width = mi.rcMonitor.right - mi.rcMonitor.left;
			int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

			::SetWindowPos(pickerMaskWindow, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top, width, height, SWP_SHOWWINDOW);
		}
	}
}

LRESULT CALLBACK ColorPicker::ColorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCREATE:
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		ColorPicker* colorPicker = (ColorPicker*)(cs->lpCreateParams);
		colorPicker->pickerColorWindow = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)colorPicker);
		return colorPicker->ColorWndHandler(hWnd, message, wParam, lParam);
	}
	default:
	{
		ColorPicker* colorPicker = reinterpret_cast<ColorPicker*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!colorPicker)
			return FALSE;
		return colorPicker->ColorWndHandler(hWnd, message, wParam, lParam);
	}
	}
}

LRESULT CALLBACK ColorPicker::ColorWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		DrawColorWnd();
	}
	break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ColorPicker::DrawColorWnd()
{
	HDC hdc_win = ::GetDC(pickerColorWindow);
	if (hdc_win) {
		RECT rc;
		HBRUSH brush;


		rc.left = 2;
		rc.top = 2;
		rc.right = ColorWindowWidth - 4;
		rc.bottom = rc.top + 22;
		brush = ::CreateSolidBrush(0);
		::FrameRect(hdc_win, &rc, brush);
		::DeleteObject(brush);

		rc.left = 3;
		rc.top = 3;
		rc.right = ColorWindowWidth - 5;
		rc.bottom = rc.top + 20;
		brush = ::CreateSolidBrush(colorPickedInfo.color);
		::FillRect(hdc_win, &rc, brush);
		::DeleteObject(brush);

		rc.left = 0;
		rc.top = 24;
		rc.right = ColorWindowWidth;
		rc.bottom = ColorWindowHeight;

		char buffer[32];
		snprintf(buffer, sizeof(buffer), "0x%08X ", colorPickedInfo.color);
		size_t len = strlen(buffer);
		WCHAR unistring[32];
		int result = MultiByteToWideChar(CP_OEMCP, 0, buffer, -1, unistring, len + 1);
		DrawText(hdc_win, (LPWSTR)unistring, -1, &rc, DT_CENTER | DT_BOTTOM);

		::ReleaseDC(pickerColorWindow, hdc_win);
	}
}

std::string ColorPicker::GetColorHex(COLORREF& ref)
{
	std::stringstream redHex;
	redHex << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast<int>(GetRValue(ref));

	std::stringstream greenHex;
	greenHex << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast<int>(GetGValue(ref));

	std::stringstream blueHex;
	blueHex << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast<int>(GetBValue(ref));

	std::stringstream colorHex;
	colorHex << redHex.str() << greenHex.str() << blueHex.str();

	return colorHex.str();
}

/* NODE FUNCTIONS */

NAN_METHOD(StartColorPicker)
{
	if (ColorPicker::IsBusy())
	{

	}
	else {
		bool showColorWindowFlag = true;
		bool sendMoveCallbacksFlag = false;

		auto* progress = new Callback(To<v8::Function>(info[0]).ToLocalChecked());
		auto* callback = new Callback(To<v8::Function>(info[1]).ToLocalChecked());
        if(info.Length() == 4)
        {
		    showColorWindowFlag = info[2]->BooleanValue();
		    sendMoveCallbacksFlag = info[3]->BooleanValue();
        }

		AsyncQueueWorker(new ColorPicker(callback, progress, showColorWindowFlag, sendMoveCallbacksFlag));
	}
}

NAN_MODULE_INIT(Init)
{
	Nan::Set(target, New<String>("startColorPicker").ToLocalChecked(), GetFunction(New<FunctionTemplate>(StartColorPicker)).ToLocalChecked());
}

NODE_MODULE(color_picker, Init)