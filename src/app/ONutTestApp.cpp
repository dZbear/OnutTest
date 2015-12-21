#include "ONutTestApp.h"

// game views
#include "StartView.h"


ONutTestApp::ONutTestApp()
{
}

ONutTestApp::~ONutTestApp()
{
}

void ONutTestApp::OnStart()
{
	// create game specific views
	AddView("StartView", new StartView());

	// show the default view
	PushView("StartView");
}

void ONutTestApp::OnUpdate()
{
    static float timer = 2.f;
    static bool hidden = false;
    timer -= ODT;
    if (timer <= 0)
    {
        timer = 1.f;
        if (!hidden)
        {
            seed::App::SendCommand(seed::eAppCommand::POP_VIEW);
        }
        else
        {
            seed::App::SendCommand(seed::eAppCommand::PUSH_VIEW, "StartView");
        }
        hidden = !hidden;
    }
}

void ONutTestApp::OnRender()
{

}



