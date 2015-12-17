#include "SeedApp.h"
#include "SeedView.h"

SeedApp::SeedApp()
{

}

SeedApp::~SeedApp()
{

}

void SeedApp::Start()
{
	OnStart();
}

void SeedApp::Update()
{
	for (SeedView* v : m_views)
	{
		v->Update();
	}
	OnUpdate();
}

void SeedApp::Render()
{
	for (SeedView* v : m_views)
	{
		v->Render();
	}
	OnRender();
}

void SeedApp::CreateView(SeedView* in_newView)
{
	in_newView->OnCreate();
	m_views.push_back(in_newView);
}



