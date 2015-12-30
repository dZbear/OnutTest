#include "Sprite.h"

namespace seed
{
    Sprite::Sprite()
        : m_texture(nullptr)
    {
        m_align = Vector2(.5f, .5f);
        m_blend = onut::SpriteBatch::eBlendMode::PreMultiplied;
        m_filter = onut::SpriteBatch::eFiltering::Linear;
        m_flippedH = false;
        m_flippedV = false;
    }

    Sprite::~Sprite()
    {

    }

    Node* Sprite::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes)
    {
        Sprite* newNode = in_pool.alloc<Sprite>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        return newNode;
    }

    void Sprite::Copy(Node* in_copy)
    {
        Node::Copy(in_copy);
        Sprite* copy = (Sprite*)in_copy;
        copy->SetAlign(GetAlign());
        copy->SetTexture(GetTexture());

        copy->SetSpriteAnim(GetSpriteAnim());
        if (m_anim.isPlaying())
        {
            copy->SetSpriteAnim(m_lastAnim);
        }
        copy->SetFilter(GetFilter());
        copy->SetBlend(GetBlend());
        copy->SetFlipped(GetFlippedH(), GetFlippedV());
    }

    void Sprite::SetSpriteAnim(OSpriteAnim in_anim)
    {
        m_anim = in_anim;
    }

    void Sprite::Render(Matrix* in_parentMatrix)
    {
        if (!m_visible)
        {
            return;
        }

        // generate our matrix
        Matrix transform = Matrix::Identity;
        transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (in_parentMatrix)
        {
            transform = transform * *in_parentMatrix;
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform);

        // render ourself
        static Color color;
        m_color.get().Premultiply(color);
        OSpriteBatch->changeBlendMode(m_blend);
        OSpriteBatch->changeFiltering(m_filter);
        if (!m_texture)
        {
            Vector4 uvs = m_anim.getUVs();
            if (m_flippedH)
            {
                std::swap(uvs.x, uvs.z);
            }
            if (m_flippedV)
            {
                std::swap(uvs.y, uvs.w);
            }
            OSpriteBatch->drawSpriteWithUVs(m_anim.getTexture(), transform, uvs, color, m_anim.getOrigin());
        }
        else
        {
            Vector4 uvs = Vector4(0, 0, 1, 1);
            if (m_flippedH)
            {
                std::swap(uvs.x, uvs.z);
            }
            if (m_flippedV)
            {
                std::swap(uvs.y, uvs.w);
            }
            OSpriteBatch->drawSpriteWithUVs(m_texture, transform, uvs, color, m_align);
        }

        // render fg children
        RenderChildren(m_fgChildren, &transform);
    }

    void Sprite::SetTexture(OTexture* in_texture)
    {
        m_texture = in_texture;
    }

    OTexture* Sprite::GetTexture() const
    {
        return m_texture;
    }

    

    void Sprite::SetAlign(const Vector2& in_align)
    {
        m_align = in_align;
    }

    const Vector2& Sprite::GetAlign()
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

    void Sprite::SetSpriteAnimSource(const string& in_sourceName)
    {
        m_anim = OSpriteAnim(in_sourceName);
    }

    void Sprite::SetSpriteAnim(const string& in_animName)
    {
        if (in_animName == m_lastAnim)
        {
            return;
        }
        m_lastAnim = in_animName;
        m_anim.start(in_animName);
    }
    
    void Sprite::StopSpriteAnim()
    {
        m_anim.stop();
    }

    void Sprite::SetFilter(onut::SpriteBatch::eFiltering in_filter)
    {
        m_filter = in_filter;
    }

    onut::SpriteBatch::eFiltering Sprite::GetFilter()
    {
        return m_filter;
    }

    void Sprite::SetBlend(onut::SpriteBatch::eBlendMode in_blend)
    {
        m_blend = in_blend;
    }

    onut::SpriteBatch::eBlendMode Sprite::GetBlend()
    {
        return m_blend;
    }

    void Sprite::SetFlipped(bool in_flipH, bool in_flipV)
    {
        m_flippedH = in_flipH;
        m_flippedV = in_flipV;
    }

    bool Sprite::GetFlippedH()
    {
        return m_flippedH;
    }

    bool Sprite::GetFlippedV()
    {
        return m_flippedV;
    }
}
