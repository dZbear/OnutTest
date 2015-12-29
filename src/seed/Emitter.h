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

        // only to be used by the seed sdk
        virtual void    Update();
        virtual void    Render(Matrix* in_parentMatrix=nullptr);

    protected:

        OEmitterInstance    m_emitter;
        string              m_fxName;

    };
}