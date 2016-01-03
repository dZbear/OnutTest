#include "ONutTestApp.h"

// game views
#include "SplashView.h"
#include "StartView.h"
#include "GameView.h"


ONutTestApp::ONutTestApp()
{
}

ONutTestApp::~ONutTestApp()
{
}

void ONutTestApp::OnStart()
{
	// create game specific views
    AddView("SplashView", new SplashView());
    AddView("StartView", new StartView());
    AddView("GameView", new GameView());

	// show the default view
	PushView("SplashView");
}

void ONutTestApp::OnUpdate()
{
}

void ONutTestApp::OnRender()
{

}



