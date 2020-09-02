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
#include <sstream>
#include <ios>
#include <iomanip>
#include <iostream>

#define MOUSECLICK_EVENT "mouseClick"
#define MOUSEMOVE_EVENT "mouseMove"
#define FINISHED_EVENT "nonEvent"

HINSTANCE hInst = NULL;
const DWORD IDT_TIMER_OVER_WINDOW = 1001;

using namespace Nan;
using namespace v8;

ColorPicker::ColorPicker(Nan::Callback* cb, Nan::Callback* event) :
        AsyncProgressQueueWorker(cb),
        m_getColorThread(),
        m_event(event)
{
    m_colorEvent = CreateEvent(nullptr, false, false, L"");
}

ColorPicker::~ColorPicker() {
    CloseHandle(m_colorEvent);
    delete m_event;
}

void ColorPicker::Execute(const AsyncProgressQueueWorker::ExecutionProgress& progress) {
    m_getColorThread = std::thread(&ColorPicker::GetPixelColorOnCursor, this);

    while (!exitWorker) {
        WaitForSingleObject(m_colorEvent, INFINITE);

        if (m_colorInfo.event.compare(FINISHED_EVENT) != 0) {
            progress.Send(&m_colorInfo, 1);
        } 
    }
    
    if (m_getColorThread.joinable()) {
        m_getColorThread.join();
    }
}

void ColorPicker::HandleProgressCallback(const Color* data, size_t size) {
    Nan::HandleScope scope;

    v8::Local<v8::Object> eventInfo = Nan::New<v8::Object>();
    Nan::Set(eventInfo, Nan::New("event").ToLocalChecked(), New<v8::String>(data->event.c_str()).ToLocalChecked());
    Nan::Set(eventInfo, Nan::New("hex").ToLocalChecked(), New<v8::String>(data->hex.c_str()).ToLocalChecked());
    v8::Local<v8::Value> argv[] = {eventInfo};

    AsyncResource resource("color-picker:Event");
    m_event->Call(1, argv, &resource);
}

void ColorPicker::GetPixelColorOnCursor() 
{
    hInst = GetModuleHandle(NULL);

//todo load coursor 
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = ColorPicker::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hCursor = hPickCursor;
	wc.lpszClassName = L"picker_win_cls";
	wc.hbrBackground = NULL;

	if (RegisterClassEx(&wc)) {
        std::cout << "created picker_win_cls class "  << std::endl;
        pick_mask_window = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, L"picker_win_cls", L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInst, this);
        if (pick_mask_window) {
            std::cout << "created picker window"  << std::endl;
            UpdateWindow(pick_mask_window);
            ShowWindow(pick_mask_window, SW_SHOW);
            
            SetTimer(pick_mask_window, IDT_TIMER_OVER_WINDOW, 100, (TIMERPROC) NULL);

            MSG msg;
            while (GetMessage(&msg, pick_mask_window, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            std::cout << "exit message loop " << std::endl;    
            KillTimer(pick_mask_window, IDT_TIMER_OVER_WINDOW);
            DestroyWindow(pick_mask_window);
        }
        UnregisterClass(L"picker_win_cls", hInst);
	} else {
        std::cout << "failed to create class "  << GetLastError()<< std::endl;
    }
    
    UnregisterClass(L"picker_win_cls", hInst);
    
    m_colorInfo.event = FINISHED_EVENT;
    exitWorker = true;
    SetEvent(m_colorEvent);
}

LRESULT CALLBACK ColorPicker::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
		case WM_NCCREATE:
		{
			CREATESTRUCT*  cs = (CREATESTRUCT*)lParam;
			ColorPicker* colorPicker = (ColorPicker*)(cs->lpCreateParams);
			colorPicker->pick_mask_window = hWnd;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)colorPicker);
			return colorPicker->ColorWndProc(hWnd, message, wParam, lParam);
		}
		default:
		{
			ColorPicker* colorPicker = reinterpret_cast<ColorPicker*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if (!colorPicker)
				return FALSE;
			return colorPicker->ColorWndProc(hWnd, message, wParam, lParam);
		}
	}
}

LRESULT CALLBACK ColorPicker::ColorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_SETCURSOR:
	{
		//SetCursor(hPickCursor);
        return 0;
	}
	case WM_MOUSEMOVE:
	{
        return 0;
	}
    case WM_TIMER: 
    
    if (wParam == IDT_TIMER_OVER_WINDOW) 
    { 
        POINT cursorPos;
        COLORREF colorRef;
        HDC screenDC;

        colorRef = CLR_INVALID;

        screenDC = GetDC(nullptr);
        if (screenDC != nullptr) {
            if (GetCursorPos(&cursorPos)) {
                colorRef = GetPixel(screenDC, cursorPos.x, cursorPos.y);
            }
            ReleaseDC(GetDesktopWindow(), screenDC);

            if (colorRef != CLR_INVALID) {
                Color colorInfo;
                colorInfo.hex = GetColorHex(colorRef);
                colorInfo.event = MOUSEMOVE_EVENT;
                m_colorInfo = colorInfo;
                SetEvent(m_colorEvent);
            }
        }


        POINT p;
        GetCursorPos(&p);

        HWND under_coursor = WindowFromPoint(p);
        if( under_coursor != pick_mask_window) {
            HMONITOR monitor = MonitorFromPoint(p, MONITOR_DEFAULTTONULL);
            if(monitor) {
                MONITORINFO mi;
                mi.cbSize = sizeof(MONITORINFO);
                ::GetMonitorInfo(monitor, (LPMONITORINFO)&mi);
                int width = mi.rcMonitor.right - mi.rcMonitor.left;
                int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

                ::SetWindowPos(pick_mask_window, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top, width, height, SWP_SHOWWINDOW);
            }
        }
    }
    return 0;
	case WM_LBUTTONUP:
        std::cout << "left button up " << std::endl;
        m_colorInfo.event = MOUSECLICK_EVENT;
        SetEvent(m_colorEvent);
		return 0;
	case WM_RBUTTONUP:
        std::cout << "right button up " << std::endl;
        PostQuitMessage(0);
        CloseWindow(pick_mask_window);
		return 0;
	case WM_CLOSE:
        std::cout << "WM_CLOSE recieved " << std::endl;
        break;
    case WM_SYSCOMMAND:
        std::cout << "WM_SYSCOMMAND recieved " << std::endl;
        if (SC_CLOSE == wParam)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        std::cout << "WM_DESTROY recieved " << std::endl;
        PostQuitMessage(0);
        break;
	default:
        break;
	}

    return DefWindowProc(hWnd, message, wParam, lParam);
}

std::string ColorPicker::GetColorHex(COLORREF &ref) {
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

NAN_METHOD(StartColorPicker) {
    auto* progress = new Callback(To<v8::Function>(info[0]).ToLocalChecked());
    auto* callback = new Callback(To<v8::Function>(info[1]).ToLocalChecked());

    AsyncQueueWorker(new ColorPicker(callback, progress));
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, New<String>("startColorPicker").ToLocalChecked(),GetFunction(New<FunctionTemplate>(StartColorPicker)).ToLocalChecked());
}

NODE_MODULE(color_picker, Init)