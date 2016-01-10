#include "Emitter.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    Emitter::Emitter()
    {
        m_blend = onut::SpriteBatch::eBlendMode::Add;
        m_filter = onut::SpriteBatch::eFiltering::Linear;
        m_emitWorld = true;
    }

    Emitter::~Emitter()
    {

    }

    Node* Emitter::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        Emitter* newNode = in_pool.alloc<Emitter>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* Emitter::Duplicate() const
    {
        Emitter* newNode = new Emitter();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void Emitter::Copy(Node* in_copy) const
    {
        Node::Copy(in_copy);
        Emitter* copy = (Emitter*)in_copy;

        copy->m_fxName = m_fxName;
        copy->m_filter = m_filter;
        copy->m_blend = m_blend;
        copy->m_emitWorld = m_emitWorld;

        if (m_emitter.isPlaying())
        {
            copy->Start();
        }
    }

    void Emitter::Init(const string& in_fxName)
    {
        m_fxName = in_fxName;
    }

    void Emitter::Update()
    {
        Node::Update();

        if (m_emitWorld)
        {
            m_emitter.setTransform(GetTransform().Translation());
        }
    }

    void Emitter::Render(Matrix* in_parentMatrix, float in_parentAlpha)
    {
        if (!m_visible)
        {
            return;
        }

        // generate our matrix
        Matrix transform = Matrix::Identity;
        transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (in_parentMatrix)
        {
            transform = transform * *in_parentMatrix;
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // render ourself
        if (m_emitWorld)
        {
            OSpriteBatch->changeBlendMode(m_blend);
            OSpriteBatch->changeFiltering(m_filter);
            m_emitter.render();
        }
        else
        {
            OSpriteBatch->end();
            Matrix spriteBatchTransform = OSpriteBatch->getTransform();
            OSpriteBatch->begin(transform * spriteBatchTransform);

            OSpriteBatch->changeBlendMode(m_blend);
            OSpriteBatch->changeFiltering(m_filter);
            m_emitter.setTransform(Vector3::Zero);
            m_emitter.render();

            OSpriteBatch->end();
            OSpriteBatch->begin(spriteBatchTransform);
        }

        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
    }

    void Emitter::Start()
    {
        if (m_emitWorld)
        {
            m_emitter = OEmitPFX(m_fxName.c_str(), GetTransform().Translation());
        }
        else
        {
            m_emitter = OEmitPFX(m_fxName.c_str(), Vector3::Zero);
        }
        m_emitter.setRenderEnabled(false);
    }

    void Emitter::Stop()
    {
        m_emitter.stop();
    }

    void Emitter::SetFilter(onut::SpriteBatch::eFiltering in_filter)
    {
        m_filter = in_filter;
    }

    onut::SpriteBatch::eFiltering Emitter::GetFilter() const
    {
        return m_filter;
    }

    void Emitter::SetBlend(onut::SpriteBatch::eBlendMode in_blend)
    {
        m_blend = in_blend;
    }

    onut::SpriteBatch::eBlendMode Emitter::GetBlend() const
    {
        return m_blend;
    }

    bool Emitter::GetEmitWorld() const
    {
        return m_emitWorld;
    }

    void Emitter::SetEmitWorld(bool in_emitWorld)
    {
        m_emitWorld = in_emitWorld;
    }

    OEmitterInstance& Emitter::GetEmitterInstance()
    {
        return m_emitter;
    }

    const string& Emitter::GetFxName() const
    {
        return m_fxName;
    }

    extern std::unordered_map<onut::SpriteBatch::eFiltering, string> filteringMap;
    extern std::unordered_map<onut::SpriteBatch::eBlendMode, string> blendMap;

    tinyxml2::XMLElement* Emitter::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("Emitter");
        xmlNode->SetAttribute("fx", GetFxName().c_str());
        xmlNode->SetAttribute("filter", filteringMap[GetFilter()].c_str());
        xmlNode->SetAttribute("blend", blendMap[GetBlend()].c_str());
        xmlNode->SetAttribute("emitWorld", m_emitWorld);

        return xmlNode;
    }

    void Emitter::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        const char* filter = in_xmlNode->Attribute("filter");
        for (auto &kv : filteringMap) if (kv.second == filter) SetFilter(kv.first);

        const char* blend = in_xmlNode->Attribute("blend");
        for (auto &kv : blendMap) if (kv.second == blend) SetBlend(kv.first);

        in_xmlNode->QueryBoolAttribute("emitWorld", &m_emitWorld);
    }
}
