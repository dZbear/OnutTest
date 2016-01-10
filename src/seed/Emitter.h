#pragma once

#include "Node.h"

namespace seed
{
    class Emitter : public Node
    {
    public:

        Emitter();
        virtual ~Emitter();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*                   Duplicate() const;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void Init(const string& in_fxName);
        void Start();
        void Stop();

        OEmitterInstance&   GetEmitterInstance();
        const string&       GetFxName() const;

        void                            SetFilter(onut::SpriteBatch::eFiltering in_filter);
        onut::SpriteBatch::eFiltering   GetFilter() const;
        void                            SetBlend(onut::SpriteBatch::eBlendMode in_blend);
        onut::SpriteBatch::eBlendMode   GetBlend() const;
        bool                            GetEmitWorld() const;
        void                            SetEmitWorld(bool in_emitWorld);

        // only to be used by the seed sdk
        virtual void    Update() override;
        virtual void    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;

    protected:

        virtual void            Copy(Node* in_copy) const;

        OEmitterInstance                m_emitter;
        string                          m_fxName;
        onut::SpriteBatch::eFiltering   m_filter;
        onut::SpriteBatch::eBlendMode   m_blend;
        bool                            m_emitWorld;

    };
}