#pragma once

#include "Sprite.h"
#include "onut.h"

namespace seed
{
    class SpriteString : public Sprite
    {
    public:

        SpriteString();
        virtual ~SpriteString();

        virtual Node*           Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*           Duplicate() const;
        tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const override;
        void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode) override;

        void            SetFont(OFont* in_font);
        OFont*          GetFont() const { return m_font; }
        void            SetCaption(const string& in_caption);
        const string&   GetCaption() const { return m_caption; }

        // only to be used by the seed sdk
        void    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;

        virtual float   GetWidth() const override;
        virtual float   GetHeight() const override;

    private:

        virtual void        Copy(Node* in_copy) const;

        string      m_caption;
        OFont*      m_font;

        const onut::Align   GetFontAlignFromSpriteAlign();
    };
}