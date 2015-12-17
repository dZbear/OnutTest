#include "SeedView.h"

SeedView::SeedView()
{

}

SeedView::~SeedView()
{

}

void SeedView::Show()
{
	OnShow();
}

void SeedView::Hide()
{
	OnHide();
}

void SeedView::Pause()
{
	OnPause();
}

void SeedView::Resume()
{
	OnResume();
}

void SeedView::Update()
{
	OnUpdate();
}

void SeedView::Render()
{
	OnRender();
}





