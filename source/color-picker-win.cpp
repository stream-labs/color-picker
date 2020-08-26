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

void ColorPicker::GetPixelColorOnCursor() {
    POINT cursorPos;
    COLORREF colorRef;
    HDC screenDC;
    int swappedMouseButton = GetSystemMetrics(SM_SWAPBUTTON);

    while (true) {
        colorRef = CLR_INVALID;
        m_colorInfo.event = FINISHED_EVENT;

        screenDC = GetDC(nullptr);
        if (screenDC == nullptr) {
            break;
        }

        if (GetCursorPos(&cursorPos)) {
            colorRef = GetPixel(screenDC, cursorPos.x, cursorPos.y);
        }
        ReleaseDC(GetDesktopWindow(), screenDC);

        if (colorRef == CLR_INVALID) {
            break;
        }

        Color colorInfo;
        colorInfo.hex = GetColorHex(colorRef);

        if ((swappedMouseButton == 0 && GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
            (swappedMouseButton != 0 && GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
            colorInfo.event = MOUSECLICK_EVENT;
            m_colorInfo = colorInfo;
            break;
        } else {
            colorInfo.event = MOUSEMOVE_EVENT;
            m_colorInfo = colorInfo;
            SetEvent(m_colorEvent);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    exitWorker = true;
    SetEvent(m_colorEvent);
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