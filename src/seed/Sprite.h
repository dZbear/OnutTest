#pragma once

#include "Node.h"

namespace seed
{
    class Sprite : public Node
    {
    public:

        Sprite();
        virtual ~Sprite();

        void            SetAnimSource( const string& in_sourceName );
        void            SetAnim(const string& in_animName);
        void            StopAnim();

        void            SetTexture(OTexture* in_texture);
        OTexture*       GetTexture() const;
        void            SetAlign(const Vector2& in_align);
        const Vector2&  GetAlign();
        void            SetFilter(onut::SpriteBatch::eFiltering in_filter);
        void            SetBlend(onut::SpriteBatch::eBlendMode in_blend);
        
        virtual float   GetWidth();
        virtual float   GetHeight();

        // only to be used by the seed sdk
        virtual void    Render(Matrix* in_parentMatrix=nullptr);

    protected:

        Vector2                         m_align;
        OTexture*                       m_texture;
        OSpriteAnim                     m_anim;
        onut::SpriteBatch::eFiltering   m_filter;
        onut::SpriteBatch::eBlendMode   m_blend;
    };
}