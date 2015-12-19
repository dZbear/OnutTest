#pragma once

#include "SeedGlobals.h"
#include "onut.h"

namespace seed
{
	class Sprite
	{
	public:

		Sprite();
		virtual ~Sprite();

		// only to be used by the seed sdk
		void Update();
		void Render();

	private:

		OAnim<Vector2>			m_position;
		OAnim<Vector2>			m_scale;
		OAnim<float>			m_angle;
		Vector2					m_align;
		
		SpriteVect		m_bgChildren;
		SpriteVect		m_fgChildren;
		Sprite*			m_parent;


	};
}