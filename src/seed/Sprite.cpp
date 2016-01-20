#include "Sprite.h"
#include "tinyxml2.h"
#include "View.h"

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

    Node* Sprite::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        Sprite* newNode = in_pool.alloc<Sprite>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* Sprite::Duplicate() const
    {
        Sprite* newNode = new Sprite();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    std::unordered_map<onut::SpriteBatch::eFiltering, string> filteringMap = {
        {onut::SpriteBatch::eFiltering::Linear, "Linear"},
        {onut::SpriteBatch::eFiltering::Nearest, "Nearest"}
    };

    std::unordered_map<onut::SpriteBatch::eBlendMode, string> blendMap = {
        {onut::SpriteBatch::eBlendMode::Add, "Add"},
        {onut::SpriteBatch::eBlendMode::Alpha, "Alpha"},
        {onut::SpriteBatch::eBlendMode::ForceWrite, "ForceWrite"},
        {onut::SpriteBatch::eBlendMode::Multiplied, "Multiplied"},
        {onut::SpriteBatch::eBlendMode::Opaque, "Opaque"},
        {onut::SpriteBatch::eBlendMode::PreMultiplied, "PreMultiplied"},
    };

    tinyxml2::XMLElement* Sprite::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement* xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("Sprite");

        xmlNode->SetAttribute("alignX", GetAlign().x);
        xmlNode->SetAttribute("alignY", GetAlign().y);
        if (GetTexture())
        {
            xmlNode->SetAttribute("texture", GetTexture()->getName().c_str());
        }
        xmlNode->SetAttribute("filter", filteringMap[GetFilter()].c_str());
        xmlNode->SetAttribute("blend", blendMap[GetBlend()].c_str());
        xmlNode->SetAttribute("flippedH", GetFlippedH());
        xmlNode->SetAttribute("flippedV", GetFlippedV());

        return xmlNode;
    }

    void Sprite::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        Vector2 align = GetAlign();
        in_xmlNode->QueryAttribute("alignX", &align.x);
        in_xmlNode->QueryAttribute("alignY", &align.y);
        SetAlign(align);

        const char* filter = in_xmlNode->Attribute("filter");
        for (auto &kv : filteringMap) if (kv.second == filter) SetFilter(kv.first);

        const char* blend = in_xmlNode->Attribute("blend");
        for (auto &kv : blendMap) if (kv.second == blend) SetBlend(kv.first);

        bool flippedH = GetFlippedH();
        in_xmlNode->QueryAttribute("flippedH", &flippedH);
        bool flippedV = GetFlippedV();
        in_xmlNode->QueryAttribute("flippedV", &flippedV);
        SetFlipped(flippedH, flippedV);
    }

    void Sprite::Copy(Node* in_copy) const
    {
        Node::Copy(in_copy);
        Sprite* copy = (Sprite*)in_copy;

        copy->m_align = m_align;
        copy->m_texture = m_texture;
        copy->m_anim = m_anim;
        copy->m_filter = m_filter;
        copy->m_blend = m_blend;
        copy->m_flippedH = m_flippedH;
        copy->m_flippedV = m_flippedV;

        if (m_anim.isPlaying())
        {
            copy->SetSpriteAnim(m_lastAnim);
        }
    }

    void Sprite::SetSpriteAnim(OSpriteAnim in_anim)
    {
        m_anim = in_anim;
    }

    void Sprite::Render(Matrix* in_parentMatrix, float in_parentAlpha)
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
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // render ourself
        Color color = m_color.get();
        color.w *= in_parentAlpha;
        color.Premultiply();
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
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
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

    const Vector2& Sprite::GetAlign() const
    {
        return m_align;
    }

    float Sprite::GetWidth() const
    {
        if (m_texture)
        {
            return (float)m_texture->getSize().x;
        }
        else if (m_anim.getTexture())
        {
            return (float)m_anim.getTexture()->getSizef().x;
        }
        return 0;
    }
    float Sprite::GetHeight() const
    {
        if (m_texture)
        {
            return (float)m_texture->getSize().y;
        }
        else if (m_anim.getTexture())
        {
            return (float)m_anim.getTexture()->getSizef().y;
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

    onut::SpriteBatch::eFiltering Sprite::GetFilter() const
    {
        return m_filter;
    }

    void Sprite::SetBlend(onut::SpriteBatch::eBlendMode in_blend)
    {
        m_blend = in_blend;
    }

    onut::SpriteBatch::eBlendMode Sprite::GetBlend() const
    {
        return m_blend;
    }

    void Sprite::SetFlipped(bool in_flipH, bool in_flipV)
    {
        m_flippedH = in_flipH;
        m_flippedV = in_flipV;
    }

    void Sprite::SetFlippedH(bool in_flipH)
    {
        m_flippedH = in_flipH;
    }

    void Sprite::SetFlippedV(bool in_flipV)
    {
        m_flippedV = in_flipV;
    }

    bool Sprite::GetFlippedH() const
    {
        return m_flippedH;
    }

    bool Sprite::GetFlippedV() const
    {
        return m_flippedV;
    }
}
