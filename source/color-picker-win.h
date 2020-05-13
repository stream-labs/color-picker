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

#include <nan.h>
#include <functional>
#include <string>
#include <mutex>
#include <thread>

struct Color {
    std::string event;
    int redValue;
    int greenValue;
    int blueValue;
    std::string hex;
};

class ColorPicker : public Nan::AsyncProgressQueueWorker<Color> {
public:
    enum THREAD_MSG {
        CLOSE
    };

    ColorPicker(Nan::Callback* cb, Nan::Callback* event);
    ~ColorPicker();

    void Execute(const AsyncProgressQueueWorker::ExecutionProgress& progress);
    void HandleProgressCallback(const Color* data, size_t size);

private:
    static void GetPixelColorOnCursor();
    static std::string GetColorHex(int red, int green, int blue);

    std::thread m_getColorThread;
    Nan::Callback* m_event;
};