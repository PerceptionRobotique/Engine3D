#include "Controller.h"

#if defined _WIN32 || defined _WIN64
XINPUT_STATE Controller::state;
#elif __unix__
int Controller::fd(-1);
js_event Controller::e;
QHash<Controller::Input, short> Controller::values;
#endif

Controller::Controller()
{
}

void Controller::update()
{
#if defined _WIN32 || defined _WIN64
    DWORD dwResult;
    ZeroMemory(&state, sizeof(XINPUT_STATE));
    dwResult = XInputGetState(0, &state);

    if( dwResult == ERROR_SUCCESS )
    {
        XInputGetState(0, &state);
    }
#elif defined __unix__
    std::ifstream js0("/dev/input/js0");
    if(fd == -1 && js0.good()) fd = open("/dev/input/js0", O_RDONLY);
    if(fd != -1 && !js0.good())
    {
        close(fd);
        fd = -1;
    }

    if(fd != -1)
    {
        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);
        FD_SET(fd, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 1;

        int readReady = select(fd + 1, &set, NULL, NULL, &timeout);
        while (readReady > 0)
        {
            if (read(fd, &e, sizeof(e)) == sizeof(e))
            {
                if (e.type & JS_EVENT_AXIS)
                {
                    switch (e.number)
                    {
                    case 0:
                        values[STICK_LEFT_X] = e.value;
                        break;

                    case 1:
                        values[STICK_LEFT_Y] = -e.value;
                        break;

                    case 2:
                        values[STICK_RIGHT_X] = e.value;
                        break;

                    case 3:
                        values[STICK_RIGHT_Y] = -e.value;
                        break;

                    case 4:
                        values[TRIGGER_RIGHT] = e.value;
                        break;

                    case 5:
                        values[TRIGGER_LEFT] = e.value;
                        break;

                    case 6:
                        values[DPAD_LEFT] = e.value > 0 ? e.value : 0;
                        values[DPAD_RIGHT] = e.value < 0 ? -e.value : 0;
                        break;

                    case 7:
                        values[DPAD_DOWN] = e.value > 0 ? e.value : 0;
                        values[DPAD_UP] = e.value < 0 ? -e.value : 0;
                        break;
                    }
                }
                else if (e.type == JS_EVENT_BUTTON)
                {
                    switch (e.number)
                    {
                    case 0:
                        values[A] = SHRT_MAX * e.value;
                        break;

                    case 1:
                        values[B] = SHRT_MAX * e.value;
                        break;

                    case 3:
                        values[X] = SHRT_MAX * e.value;
                        break;

                    case 4:
                        values[Y] = SHRT_MAX * e.value;
                        break;

                    case 6:
                        values[L] = SHRT_MAX * e.value;
                        break;

                    case 7:
                        values[R] = SHRT_MAX * e.value;
                        break;

                    case 11:
                        values[START] = SHRT_MAX * e.value;
                        break;

                    case 12:
                        values[XBOX] = SHRT_MAX * e.value;
                        break;

                    case 13:
                        values[STICK_LEFT_BUTTON] = SHRT_MAX * e.value;
                        break;

                    case 14:
                        values[STICK_RIGHT_BUTTON] = SHRT_MAX * e.value;
                        break;
                    }
                }
            }
            readReady = select(fd + 1, &set, NULL, NULL, &timeout);
        }
    }
#endif
}

float Controller::getInput(Input input)
{
    float value = 0;
#if defined _WIN32 || defined _WIN64
    switch(input)
    {
    case STICK_LEFT_X:
        value = std::abs(state.Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? ((float)state.Gamepad.sThumbLX)/((float)SHRT_MAX) : 0;
        break;

    case STICK_LEFT_Y:
        value = std::abs(state.Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? ((float)state.Gamepad.sThumbLY)/((float)SHRT_MAX) : 0;
        break;

    case STICK_RIGHT_X:
        value = std::abs(state.Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? ((float)state.Gamepad.sThumbRX)/((float)SHRT_MAX) : 0;
        break;

    case STICK_RIGHT_Y:
        value = std::abs(state.Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? ((float)state.Gamepad.sThumbRY)/((float)SHRT_MAX) : 0;
        break;

    case TRIGGER_LEFT:
        value = state.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? ((float)state.Gamepad.bLeftTrigger)/((float)UCHAR_MAX) : 0;
        break;

    case TRIGGER_RIGHT:
        value = state.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? ((float)state.Gamepad.bRightTrigger)/((float)UCHAR_MAX) : 0;
        break;

    case DPAD_UP:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) > 0);
        break;

    case DPAD_DOWN:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) > 0);
        break;

    case DPAD_LEFT:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) > 0);
        break;

    case DPAD_RIGHT:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) > 0);
        break;

    case A:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) > 0);
        break;

    case B:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) > 0);
        break;

    case X:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_X) > 0);
        break;

    case Y:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) > 0);
        break;

    case L:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0);
        break;

    case R:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0);
        break;

    case BACK:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) > 0);
        break;

    case START:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_START) > 0);
        break;

    case STICK_LEFT_BUTTON:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) > 0);
        break;

    case STICK_RIGHT_BUTTON:
        value = (float)((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) > 0);
        break;
    }

#elif defined __unix__
    value = ((float)values[input])/((float)SHRT_MAX);
#endif
    return value;
}
