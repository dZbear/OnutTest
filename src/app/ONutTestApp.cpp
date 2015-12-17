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
	CreateView(new StartView());
}

void ONutTestApp::OnUpdate()
{
	
}

void ONutTestApp::OnRender()
{

}



