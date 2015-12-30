#pragma once

#include "Node.h"

namespace seed
{
    class Emitter : public Node
    {
    public:

        Emitter();
        virtual ~Emitter();

        virtual Node*   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes);

        void Init(const string& in_fxName);
        void Start();
        void Stop();

        OEmitterInstance&   GetEmitterInstance();
        string              GetFxName();

        void                            SetFilter(onut::SpriteBatch::eFiltering in_filter);
        onut::SpriteBatch::eFiltering   GetFilter();
        void                            SetBlend(onut::SpriteBatch::eBlendMode in_blend);
        onut::SpriteBatch::eBlendMode   GetBlend();

        // only to be used by the seed sdk
        virtual void    Update();
        virtual void    Render(Matrix* in_parentMatrix=nullptr);

    protected:

        virtual void            Copy(Node* in_copy);

        OEmitterInstance                m_emitter;
        string                          m_fxName;
        onut::SpriteBatch::eFiltering   m_filter;
        onut::SpriteBatch::eBlendMode   m_blend;

    };
}