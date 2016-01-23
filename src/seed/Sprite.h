#pragma once

#include "Node.h"

namespace seed
{
    class Sprite : public Node
    {
    public:

        Sprite();
        virtual ~Sprite();

        virtual Node*           Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*           Duplicate() const;
        tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const override;
        void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode) override;

        void            SetSpriteAnimSource(const string& in_sourceName);
        void            SetSpriteAnim(const string& in_animName);
        void            SetSpriteAnim(OSpriteAnim in_anim);
        void            StopSpriteAnim();
        OSpriteAnim&    GetSpriteAnim() { return m_anim; }

        void            SetTexture(OTexture* in_texture);
        OTexture*       GetTexture() const;
        void            SetAlign(const Vector2& in_align);
        const Vector2&  GetAlign() const;
        void            SetFlipped(bool in_flipH, bool in_flipV);
        void            SetFlippedH(bool in_flipH);
        void            SetFlippedV(bool in_flipV);
        bool            GetFlippedH() const;
        bool            GetFlippedV() const;
        
        virtual float   GetWidth() const override;
        virtual float   GetHeight() const override;

        void                            SetFilter(onut::SpriteBatch::eFiltering in_filter);
        onut::SpriteBatch::eFiltering   GetFilter() const;
        void                            SetBlend(onut::SpriteBatch::eBlendMode in_blend);
        onut::SpriteBatch::eBlendMode   GetBlend() const;


        // only to be used by the seed sdk
        virtual void    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;



    protected:

        virtual void            Copy(Node* in_copy) const;

        Vector2                         m_align;
        OTexture*                       m_texture = nullptr;
        OSpriteAnim                     m_anim;
        onut::SpriteBatch::eFiltering   m_filter;
        onut::SpriteBatch::eBlendMode   m_blend;
        bool                            m_flippedH;
        bool                            m_flippedV;
        string                          m_lastAnim;
    };
}