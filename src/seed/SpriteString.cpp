#include "SpriteString.h"
#include "tinyxml2.h"

namespace seed
{
    SpriteString::SpriteString()
        : m_font(nullptr)
    {
    }

    SpriteString::~SpriteString()
    {

    }

    Node* SpriteString::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        SpriteString* newNode = in_pool.alloc<SpriteString>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* SpriteString::Duplicate() const
    {
        SpriteString* newNode = new SpriteString();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void SpriteString::Copy(Node* in_copy) const
    {
        Sprite::Copy(in_copy);
        SpriteString* copy = (SpriteString*)in_copy;

        copy->m_font = m_font;
        copy->m_caption = m_caption;
    }

    void SpriteString::Render(Matrix* in_parentMatrix, float in_parentAlpha)
    {
        if (!m_visible)
        {
            return;
        }

        // generate our matrix
        Matrix transform = Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (in_parentMatrix)
        {
            transform = transform * *in_parentMatrix;
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // render the string
        if (m_font && m_caption.length() > 0)
        {
            OSpriteBatch->end();
            Matrix spriteBatchTransform = OSpriteBatch->getTransform();
            OSpriteBatch->begin(transform * spriteBatchTransform);
            
            OSpriteBatch->changeBlendMode(m_blend);
            OSpriteBatch->changeFiltering(m_filter);

            Color color = m_color;
            color.w *= in_parentAlpha;
            if (m_blend == onut::SpriteBatch::eBlendMode::PreMultiplied) color.Premultiply();
            m_font->draw(m_caption, Vector2::Zero, color, OSpriteBatch, GetAlign());
            OSpriteBatch->end();
            OSpriteBatch->begin(spriteBatchTransform);
        }

        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
    }

    void SpriteString::SetFont(OFont* in_font)
    {
        m_font = in_font;
    }

    void SpriteString::SetCaption(const string& in_caption)
    {
        m_caption = in_caption;
    }

    tinyxml2::XMLElement* SpriteString::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement* xmlNode = Sprite::Serialize(in_xmlDoc);

        xmlNode->SetName("SpriteString");

        if (GetFont())
        {
            xmlNode->SetAttribute("font", GetFont()->getName().c_str());
        }
        xmlNode->SetAttribute("caption", GetCaption().c_str());

        return xmlNode;
    }

    void SpriteString::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Sprite::Deserialize(view, in_xmlNode);

        const char* szFont = in_xmlNode->Attribute("font");
        const char* szCaption = in_xmlNode->Attribute("caption");

        if (szFont) SetFont(OGetBMFont(szFont));
        else SetFont(nullptr);

        if (szCaption) SetCaption(szCaption);
        else SetCaption("");
    }

    float SpriteString::GetWidth() const
    {
        if (m_font)
        {
            return m_font->measure(m_caption).x;
        }
        return 0;
    }

    float SpriteString::GetHeight() const
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





