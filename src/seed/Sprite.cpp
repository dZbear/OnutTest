#include "Sprite.h"

namespace seed
{
	Sprite::Sprite()
		: m_texture(nullptr)
	{
        m_align = Vector2(.5f, .5f);
	}

	Sprite::~Sprite()
	{

	}

	void Sprite::Update()
	{
        Node::Update();

		// todo : update frame based anims
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

	void Sprite::SetTexture(OTexture* in_texture)
	{
		m_texture = in_texture;
	}

	OTexture* Sprite::GetTexture()
	{
		return m_texture;
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





