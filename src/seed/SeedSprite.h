#pragma once

#include "SeedGlobals.h"

class SeedSprite
{
public:

	SeedSprite();
	virtual ~SeedSprite();



	// only to be used by the seed sdk
	void Update();
	void Render();

private:

	Vector2			m_position;
	Vector2			m_align;
	Vector2			m_scale;

	SpriteVect		m_bgChildren;
	SpriteVect		m_fgChildren;
	SeedSprite*		m_parent;


};