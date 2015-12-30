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

        virtual Node*   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes);

        void    SetFont(OFont* in_font);
        void    SetCaption(const string& in_caption);

        // only to be used by the seed sdk
        void    Render(Matrix* in_parentMatrix=nullptr);

        virtual float    GetWidth();
        virtual float   GetHeight();

    private:

        virtual void            Copy(Node* in_copy);

        string      m_caption;
        OFont*      m_font;

        const onut::Align     GetFontAlignFromSpriteAlign();
    };
}