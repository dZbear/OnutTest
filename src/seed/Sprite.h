#pragma once

#include "Node.h"

namespace seed
{
	class Sprite : public Node
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

		// only to be used by the seed sdk
		void    Update();
		void    Render(Matrix* in_parentMatrix=nullptr);

    private:

		OAnim<Vector2>			m_position;
		OAnim<Vector2>			m_scale;
		OAnim<float>			m_angle;
        OAnim<Color>            m_color;
		Vector2					m_align;
		OTexture*				m_texture;
	};
}