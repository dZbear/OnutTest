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

		void			SetTexture(OTexture* in_texture);
		OTexture*		GetTexture();
		void			SetPosition(const Vector2& in_position);
		void			SetPosition(float in_x, float in_y);
		Vector2			GetPosition();
		OAnim<Vector2>&	GetPositionAnim();
		void			SetScale(const Vector2& in_scale);
		Vector2			GetScale();
		OAnim<Vector2>&	GetScaleAnim();
		void			SetAngle(float in_angle);
		float			GetAngle();
		OAnim<float>&	GetAngleAnim();
        void			SetColor(const Color& in_color);
        Color			GetColor();
        OAnim<Color>&	GetColorAnim();
        void            SetAlign(const Vector2& in_align);
        Vector2&        GetAlign();

		
		float		GetWidth();
		float		GetHeight();

		void		Attach(Sprite* in_newChild, int in_zIndex = INT_MAX);
		void		Detach(Sprite* in_child);
		Sprite*		GetParent();

		// only to be used by the seed sdk
		void    Update();
		void    Render(Matrix* in_parentMatrix=nullptr);
		void    RenderChildren(SpriteVect& in_children, Matrix* in_parentMatrix=nullptr);
		void    SetParent(Sprite* in_parent);

        void    SetZindex(int in_zIndex);
        int     GetZindex();


	private:

		OAnim<Vector2>			m_position;
		OAnim<Vector2>			m_scale;
		OAnim<float>			m_angle;
        OAnim<Color>            m_color;
		Vector2					m_align;

		OTexture*				m_texture;
		
		SpriteVect				m_bgChildren;
		SpriteVect				m_fgChildren;
		Sprite*					m_parent;

		int						m_zIndex;

		void					InsertSprite(SpriteVect& in_vect, Sprite* in_sprite, int in_zIndex);
		void					DetachChild(SpriteVect& in_vect, Sprite* in_sprite);
	};
}