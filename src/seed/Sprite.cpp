#include "Sprite.h"

namespace seed
{
	Sprite::Sprite()
		: m_texture(nullptr)
	{
		m_scale = Vector2(1.f, 1.f);
		m_angle = 0;
        m_color = Color(1.f, 1.f, 1.f, 1.f);
        m_align = Vector2(.5f, .5f);
	}

	Sprite::~Sprite()
	{

	}

	void Sprite::Update()
	{
		// update anims
	}

	void Sprite::Render(Matrix* in_parentMatrix)
	{
		// generate our matrix
		Matrix transform = Matrix::Identity;
		transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
		transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
		transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

		if (in_parentMatrix)
		{
			transform = transform * *in_parentMatrix;
		}

		// render bg children
		RenderChildren(m_bgChildren, &transform);

		// render ourself
		OSpriteBatch->drawSprite(m_texture, transform, m_color, m_align);

		// render fg children
        RenderChildren(m_fgChildren, &transform);
	}

	void Sprite::RenderChildren(NodeVect& in_children, Matrix* in_parentMatrix)
	{
		for (Node* node : in_children)
		{
            node->Render(in_parentMatrix);
		}
	}

	void Sprite::SetTexture(OTexture* in_texture)
	{
		m_texture = in_texture;
	}

	OTexture* Sprite::GetTexture()
	{
		return m_texture;
	}

	void Sprite::SetPosition(const Vector2& in_position)
	{
		m_position = in_position;
	}

	void Sprite::SetPosition(float in_x, float in_y)
	{
		SetPosition(Vector2(in_x, in_y));
	}

	Vector2 Sprite::GetPosition()
	{
		return m_position.get();
	}

	OAnim<Vector2>& Sprite::GetPositionAnim()
	{
		return m_position;
	}

	void Sprite::SetScale(const Vector2& in_scale)
	{
		m_scale = in_scale;
	}

	Vector2 Sprite::GetScale()
	{
		return m_scale.get();
	}

	OAnim<Vector2>& Sprite::GetScaleAnim()
	{
		return m_scale;
	}

	void Sprite::SetAngle(float in_angle)
	{
		m_angle = in_angle;
	}

	float Sprite::GetAngle()
	{
		return m_angle.get();
	}

	OAnim<float>& Sprite::GetAngleAnim()
	{
		return m_angle;
	}

    void Sprite::SetColor(const Color& in_color)
    {
        m_color = in_color;
    }

    Color Sprite::GetColor()
    {
        return m_color.get();
    }

    OAnim<Color>& Sprite::GetColorAnim()
    {
        return m_color;
    }

    void Sprite::SetAlign(const Vector2& in_align)
    {
        m_align = in_align;
    }

    Vector2& Sprite::GetAlign()
    {
        return m_align;
    }


	

	float Sprite::GetWidth()
	{
		if (m_texture)
		{
			return (float)m_texture->getSize().x;
		}
		return 0;
	}
	float Sprite::GetHeight()
	{
		if (m_texture)
		{
			return (float)m_texture->getSize().y;
		}
		return 0;
	}
}





