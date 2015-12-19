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
	
}

void ONutTestApp::OnRender()
{

}



