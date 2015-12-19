#pragma once
#include "SeedGlobals.h"

namespace seed
{
	class Sprite;
	class View
	{
	public:

		View();
		virtual ~View();

		// to be overriden by your "Game Specific" View
		virtual void OnUpdate() {};
		virtual void OnRender() {};
		virtual void OnShow() {};
		virtual void OnHide() {};
		virtual void OnPause() {};
		virtual void OnResume() {};
		virtual void OnCreate() {};
		/////

		// used exclusively by the SeedApp
		void Update();
		void Render();
		void Show();
		void Hide();
		void Pause();
		void Resume();

	private:

		// for quick reference by name
		SpriteMap		m_namedSprites;

		// actual sprites rendered by this view
		SpriteVect		m_sprites;
	};
}