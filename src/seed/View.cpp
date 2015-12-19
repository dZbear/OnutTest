#include "View.h"
#include "Sprite.h"

namespace seed
{
	View::View()
	{

	}

	View::~View()
	{

	}

	void View::Show()
	{
		OnShow();
	}

	void View::Hide()
	{
		OnHide();
	}

	void View::Pause()
	{
		OnPause();
	}

	void View::Resume()
	{
		OnResume();
	}

	void View::Update()
	{
		OnUpdate();
	}

	void View::Render()
	{
		// render sprites
		for (Sprite* s : m_sprites)
		{
			s->Render();
		}
		OnRender();
	}
}





