#include "SpriteString.h"

namespace seed
{
    SpriteString::SpriteString()
        : m_font(nullptr)
    {
    }

    SpriteString::~SpriteString()
    {

    }

    Node* SpriteString::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes)
    {
        SpriteString* newNode = in_pool.alloc<SpriteString>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        return newNode;
    }

    void SpriteString::Copy(Node* in_copy)
    {
        Sprite::Copy(in_copy);
        SpriteString* copy = (SpriteString*)in_copy;
        copy->SetFont(m_font);
        copy->SetCaption(m_caption);
    }


    void SpriteString::Render(Matrix* in_parentMatrix)
    {
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

        // render the string
        if (m_font && m_caption.length() > 0)
        {
            OSpriteBatch->end();
            OSpriteBatch->begin(transform);

            OSpriteBatch->changeBlendMode(m_blend);
            OSpriteBatch->changeFiltering(m_filter);

            m_font->draw(m_caption, { 0, 0 }, m_color, OSpriteBatch, GetFontAlignFromSpriteAlign());
            OSpriteBatch->end();
            OSpriteBatch->begin();
        }

        // render fg children
        RenderChildren(m_fgChildren, &transform);
    }

    void SpriteString::SetFont(OFont* in_font)
    {
        m_font = in_font;
    }

    void SpriteString::SetCaption(const string& in_caption)
    {
        m_caption = in_caption;
    }

    float SpriteString::GetWidth()
    {
        if (m_font)
        {
            return m_font->measure(m_caption).x;
        }
        return 0;
    }

    float SpriteString::GetHeight()
    {
        if (m_font)
        {
            return m_font->measure(m_caption).y;
        }
        return 0;
    }

    const onut::Align SpriteString::GetFontAlignFromSpriteAlign()
    {
        if (m_align.x == 0.f    &&      m_align.y == 0.f)       return onut::Align::TOP_LEFT;
        if (m_align.x == .5f    &&      m_align.y == 0.f)       return onut::Align::TOP;
        if (m_align.x == 1.f    &&      m_align.y == 0.f)       return onut::Align::TOP_RIGHT;
        if (m_align.x == 0.f    &&      m_align.y == .5f)       return onut::Align::LEFT;
        if (m_align.x == .5f    &&      m_align.y == .5f)       return onut::Align::CENTER;
        if (m_align.x == .5f    &&      m_align.y == 1.f)       return onut::Align::RIGHT;
        if (m_align.x == 0.f    &&      m_align.y == 1.f)       return onut::Align::BOTTOM_LEFT;
        if (m_align.x == .5f    &&      m_align.y == 1.f)       return onut::Align::BOTTOM;
        if (m_align.x == 1.f    &&      m_align.y == 1.f)       return onut::Align::BOTTOM_RIGHT;
        return onut::Align::CENTER;
    }
}





