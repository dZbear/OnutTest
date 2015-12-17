#pragma once
#include "SeedGlobals.h"


class SeedSprite;
class SeedView
{
public:

	SeedView();
	virtual ~SeedView();

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

	SpriteMap		m_namedSprites;
	SpriteVect		m_sprites;
};