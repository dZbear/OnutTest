#pragma once

#include "Node.h"

namespace seed
{
    class Emitter : public Node
    {
    public:

        Emitter();
        virtual ~Emitter();

        void Init(const string& in_fxName);
        void Start();
        void Stop();

        void SetFilter(onut::SpriteBatch::eFiltering in_filter);
        void SetBlend(onut::SpriteBatch::eBlendMode in_blend);

        // only to be used by the seed sdk
        virtual void    Update();
        virtual void    Render(Matrix* in_parentMatrix=nullptr);

    protected:

        OEmitterInstance                m_emitter;
        string                          m_fxName;
        onut::SpriteBatch::eFiltering   m_filter;
        onut::SpriteBatch::eBlendMode   m_blend;

    };
}