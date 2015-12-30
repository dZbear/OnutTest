#pragma once

#include "Node.h"

namespace seed
{
    class Sprite : public Node
    {
    public:

        Sprite();
        virtual ~Sprite();

        virtual Node*   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes);

        void            SetSpriteAnimSource( const string& in_sourceName );
        void            SetSpriteAnim(const string& in_animName);
        void            SetSpriteAnim(OSpriteAnim in_anim);
        void            StopSpriteAnim();
        OSpriteAnim&    GetSpriteAnim() { return m_anim; }

        void            SetTexture(OTexture* in_texture);
        OTexture*       GetTexture() const;
        void            SetAlign(const Vector2& in_align);
        const Vector2&  GetAlign();
        void            SetFlipped(bool in_flipH, bool in_flipV);
        bool            GetFlippedH();
        bool            GetFlippedV();
        
        virtual float   GetWidth();
        virtual float   GetHeight();

        void                            SetFilter(onut::SpriteBatch::eFiltering in_filter);
        onut::SpriteBatch::eFiltering   GetFilter();
        void                            SetBlend(onut::SpriteBatch::eBlendMode in_blend);
        onut::SpriteBatch::eBlendMode   GetBlend();


        // only to be used by the seed sdk
        virtual void    Render(Matrix* in_parentMatrix=nullptr);



    protected:

        virtual void            Copy(Node* in_copy);

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