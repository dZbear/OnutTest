#pragma once

#include "Node.h"

namespace seed
{
    class Sprite : public Node
    {
    public:

        Sprite();
        virtual ~Sprite();

        void            SetTexture(OTexture* in_texture);
        OTexture*       GetTexture() const;
        void            SetAlign(const Vector2& in_align);
        const Vector2&  GetAlign();
        
        virtual float   GetWidth();
        virtual float   GetHeight();

        // only to be used by the seed sdk
        virtual void    Update();
        virtual void    Render(Matrix* in_parentMatrix=nullptr);

    protected:

        Vector2         m_align;
        OTexture*       m_texture;
    };
}