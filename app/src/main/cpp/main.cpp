#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

#include "Logger.h"
#include "Renderer.h"

extern "C"
{
#include <game-activity/native_app_glue/android_native_app_glue.c>

    void on_app_cmd(android_app *app, int32_t cmd)
    {
        switch (cmd) {
            case APP_CMD_INIT_WINDOW:
                LOG_INFO("Initialing the window");
                app->userData = new Renderer(app);
                break;

            case APP_CMD_TERM_WINDOW:
            {
                LOG_INFO("Terminating the window");
                auto *renderer = (Renderer*)app->userData;
                if(renderer)
                {
                    delete renderer;
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void android_main(android_app *app)
    {
        app->onAppCmd = on_app_cmd;

        android_poll_source *pollSource;
        int events;

        do{
            if(ALooper_pollOnce(0, nullptr, &events, (void**)&pollSource) >= 0)
            {
                if(pollSource)
                {
                    pollSource->process(app, pollSource);
                }
            }

            if(!app->userData) continue;
            Renderer* renderer = (Renderer *)app->userData;
            renderer->Do_Frame();

        } while(!app->destroyRequested);
    }

}