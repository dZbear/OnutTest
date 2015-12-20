#pragma once
#include "SeedGlobals.h"
#include "onut.h"

namespace seed
{
	class Sprite;
	class View
	{
	public:

		View();
		virtual ~View();

		Sprite*	AddSprite(const string& in_textureName, int in_zIndex=INT_MAX);
		void	DeleteSprite(Sprite* in_sprite);

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

		// actual sprites rendered by this view
		SpriteVect		m_sprites;
		void			InsertSprite(Sprite* in_sprite, int in_zIndex);

		// sprite pool
		onut::Pool<true>			m_spritePool;


	};
}