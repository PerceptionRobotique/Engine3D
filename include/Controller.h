#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Engine3D_global.h"

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#include <Xinput.h>
#elif defined __unix__
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <QHash>
#include <QDebug>
#endif
#include <cmath>

class ENGINE3D_EXPORT Controller
{
public:
    enum Input{
        STICK_LEFT_X,
        STICK_LEFT_Y,
        STICK_RIGHT_X,
        STICK_RIGHT_Y,
        TRIGGER_LEFT,
        TRIGGER_RIGHT,
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        A,
        B,
        X,
        Y,
        L,
        R,
        BACK,
        START,
        STICK_LEFT_BUTTON,
        STICK_RIGHT_BUTTON,
        XBOX
    };

    Controller();

    static void update();
    static float getInput(Input input);

private:
#if defined _WIN32 || defined _WIN64
    static XINPUT_STATE state;
#elif defined __unix__
    static int fd;
    static js_event e;
    static QHash<Input, short> values;
//    QThread* controllerUpdaterThread;
#endif
};

#endif // CONTROLLER_H
