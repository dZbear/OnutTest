#include "Effect.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    Effect::Effect()
        : m_effectTarget(nullptr)
    {
    }

    Effect::~Effect()
    {
        if (m_effectTarget)
        {
            delete m_effectTarget;
        }
    }

    Node* Effect::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        Effect* newNode = in_pool.alloc<Effect>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* Effect::Duplicate() const
    {
        Effect* newNode = new Effect;
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void Effect::Copy(Node* in_copy) const
    {
        Node::Copy(in_copy);
        Effect* copy = (Effect*)in_copy;

        copy->m_blurEnabled = m_blurEnabled;
        copy->m_blurAmount = m_blurAmount;

        copy->m_sepiaEnabled = m_sepiaEnabled;
        copy->m_sepiaTone = m_sepiaTone;
        copy->m_sepiaSaturation = m_sepiaSaturation;
        copy->m_sepiaAmount = m_sepiaAmount;

        copy->m_crtEnabled = m_crtEnabled;

        copy->m_cartoonEnabled = m_cartoonEnabled;
        copy->m_cartoonTone = m_cartoonTone;

        copy->m_vignetteEnabled = m_vignetteEnabled;
        copy->m_vignetteAmount = m_vignetteAmount;
    }

    void Effect::Render(Matrix* in_parentMatrix, float in_parentAlpha)
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

        bool hasEffect = m_blurEnabled || m_sepiaEnabled || m_crtEnabled || m_cartoonEnabled || m_vignetteEnabled;

        if (!m_effectTarget && hasEffect)
        {
            m_effectTarget = OTexture::createScreenRenderTarget(true);
        }
        if (m_effectTarget && !hasEffect)
        {
            delete m_effectTarget;
            m_effectTarget = nullptr;
        }

        bool scissorEnabled = ORenderer->getScissorEnabled();
        Rect scissorRect = ORenderer->getScissor();

        if (hasEffect)
        {
            OSB->end();
            const Matrix& spriteBatchTransform = OSB->getTransform();
            m_effectTarget->bindRenderTarget();
            m_effectTarget->clearRenderTarget(Color::Transparent);
            OSB->begin(spriteBatchTransform);
            if (scissorEnabled)
            {
                ORenderer->setScissor(scissorEnabled, scissorRect);
            }
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);

        if (hasEffect)
        {
            OSB->end();
            Matrix spriteBatchTransform = OSB->getTransform();
            m_effectTarget->unbindRenderTarget();

            // Apply the effects
            if (m_cartoonEnabled)
            {
                m_effectTarget->cartoon(m_cartoonTone);
            }
            if (m_sepiaEnabled)
            {
                m_effectTarget->sepia(m_sepiaTone, m_sepiaSaturation, m_sepiaAmount);
            }
            if (m_blurEnabled)
            {
                m_effectTarget->blur(m_blurAmount);
            }
            if (m_vignetteEnabled)
            {
                m_effectTarget->vignette(m_vignetteAmount);
            }
            if (m_crtEnabled)
            {
                m_effectTarget->crt();
            }

            OSB->begin();
            if (scissorEnabled)
            {
                ORenderer->setScissor(scissorEnabled, scissorRect);
            }
            OSB->drawRect(m_effectTarget, ORectFullScreen);
            OSB->end();
            OSB->begin(spriteBatchTransform);
            if (scissorEnabled)
            {
                ORenderer->setScissor(scissorEnabled, scissorRect);
            }
        }
    }

    tinyxml2::XMLElement* Effect::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("Effect");

        tinyxml2::XMLElement *xmlBlur = in_xmlDoc->NewElement("Filter");
        xmlBlur->SetAttribute("type", "blur");
        xmlBlur->SetAttribute("enabled", m_blurEnabled);
        xmlBlur->SetAttribute("amount", m_blurAmount);
        xmlNode->InsertEndChild(xmlBlur);

        tinyxml2::XMLElement *xmlSepia = in_xmlDoc->NewElement("Filter");
        xmlSepia->SetAttribute("type", "sepia");
        xmlSepia->SetAttribute("enabled", m_sepiaEnabled);
        xmlSepia->SetAttribute("toneR", m_sepiaTone.get().x);
        xmlSepia->SetAttribute("toneG", m_sepiaTone.get().y);
        xmlSepia->SetAttribute("toneB", m_sepiaTone.get().z);
        xmlSepia->SetAttribute("saturation", m_sepiaSaturation);
        xmlSepia->SetAttribute("amount", m_sepiaAmount);
        xmlNode->InsertEndChild(xmlSepia);

        tinyxml2::XMLElement *xmlCrt = in_xmlDoc->NewElement("Filter");
        xmlCrt->SetAttribute("type", "crt");
        xmlCrt->SetAttribute("enabled", m_crtEnabled);
        xmlNode->InsertEndChild(xmlCrt);

        tinyxml2::XMLElement *xmlCartoon = in_xmlDoc->NewElement("Filter");
        xmlCartoon->SetAttribute("type", "cartoon");
        xmlCartoon->SetAttribute("enabled", m_cartoonEnabled);
        xmlCartoon->SetAttribute("toneR", m_cartoonTone.get().x);
        xmlCartoon->SetAttribute("toneG", m_cartoonTone.get().y);
        xmlCartoon->SetAttribute("toneB", m_cartoonTone.get().z);
        xmlNode->InsertEndChild(xmlCartoon);

        tinyxml2::XMLElement *xmlVignette = in_xmlDoc->NewElement("Filter");
        xmlVignette->SetAttribute("type", "vignette");
        xmlVignette->SetAttribute("enabled", m_vignetteEnabled);
        xmlVignette->SetAttribute("amount", m_vignetteAmount);
        xmlNode->InsertEndChild(xmlVignette);

        return xmlNode;
    }

    void Effect::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        for (auto pXmlChild = in_xmlNode->FirstChildElement(); pXmlChild; pXmlChild = pXmlChild->NextSiblingElement())
        {
            string childName = pXmlChild->Name();
            if (childName != "Filter") continue;

            const char* szType = pXmlChild->Attribute("type");
            if (!szType) continue;
            string typeName = szType;

            if (typeName == "blur")
            {
                m_blurEnabled = pXmlChild->BoolAttribute("enabled");

                float amount = GetBlurAmount();
                pXmlChild->QueryFloatAttribute("amount", &amount);
                SetBlurAmount(amount);
            }
            if (typeName == "sepia")
            {
                m_sepiaEnabled = pXmlChild->BoolAttribute("enabled");

                Vector3 tone = GetSepiaTone();
                pXmlChild->QueryFloatAttribute("toneR", &tone.x);
                pXmlChild->QueryFloatAttribute("toneG", &tone.y);
                pXmlChild->QueryFloatAttribute("toneB", &tone.z);
                SetSepiaTone(tone);

                float saturation = GetSepiaSaturation();
                pXmlChild->QueryFloatAttribute("saturation", &saturation);
                SetSepiaSaturation(saturation);

                float amount = GetSepiaAmount();
                pXmlChild->QueryFloatAttribute("amount", &amount);
                SetSepiaAmount(amount);
            }
            if (typeName == "crt")
            {
                m_crtEnabled = pXmlChild->BoolAttribute("enabled");
            }
            if (typeName == "cartoon")
            {
                m_cartoonEnabled = pXmlChild->BoolAttribute("enabled");

                Vector3 tone = GetCartoonTone();
                pXmlChild->QueryFloatAttribute("toneR", &tone.x);
                pXmlChild->QueryFloatAttribute("toneG", &tone.y);
                pXmlChild->QueryFloatAttribute("toneB", &tone.z);
                SetCartoonTone(tone);
            }
            if (typeName == "vignette")
            {
                m_vignetteEnabled = pXmlChild->BoolAttribute("enabled");

                float amount = GetVignetteAmount();
                pXmlChild->QueryFloatAttribute("amount", &amount);
                SetVignetteAmount(amount);
            }
        }
    }

    bool Effect::GetBlurEnabled() const
    {
        return m_blurEnabled.get();
    }

    OAnimb& Effect::GetBlurEnabledAnim()
    {
        return m_blurEnabled;
    }

    void Effect::SetBlurEnabled(bool enabled)
    {
        m_blurEnabled = enabled;
    }

    float Effect::GetBlurAmount() const
    {
        return m_blurAmount.get();
    }

    OAnimf& Effect::GetBlurAmountAnim()
    {
        return m_blurAmount;
    }

    void Effect::SetBlurAmount(float amount)
    {
        m_blurAmount = amount;
    }

    bool Effect::GetSepiaEnabled() const
    {
        return m_sepiaEnabled.get();
    }

    OAnimb& Effect::GetSepiaEnabledAnim()
    {
        return m_sepiaEnabled;
    }

    void Effect::SetSepiaEnabled(bool enabled)
    {
        m_sepiaEnabled = enabled;
    }

    const Vector3& Effect::GetSepiaTone() const
    {
        return m_sepiaTone.get();
    }

    OAnim3& Effect::GetSepiaToneAnim()
    {
        return m_sepiaTone;
    }

    void Effect::SetSepiaTone(const Vector3& tone)
    {
        m_sepiaTone = tone;
    }

    float Effect::GetSepiaSaturation() const
    {
        return m_sepiaSaturation.get();
    }

    OAnimf& Effect::GetSepiaSaturationAnim()
    {
        return m_sepiaSaturation;
    }

    void Effect::SetSepiaSaturation(float saturation)
    {
        m_sepiaSaturation = saturation;
    }

    float Effect::GetSepiaAmount() const
    {
        return m_sepiaAmount.get();
    }

    OAnimf& Effect::GetSepiaAmountAnim()
    {
        return m_sepiaAmount;
    }

    void Effect::SetSepiaAmount(float amount)
    {
        m_sepiaAmount = amount;
    }

    bool Effect::GetCrtEnabled() const
    {
        return m_crtEnabled.get();
    }

    OAnimb& Effect::GetCrtEnabledAnim()
    {
        return m_crtEnabled;
    }

    void Effect::SetCrtEnabled(bool enabled)
    {
        m_crtEnabled = enabled;
    }

    bool Effect::GetCartoonEnabled() const
    {
        return m_cartoonEnabled.get();
    }

    OAnimb& Effect::GetCartoonEnabledAnim()
    {
        return m_cartoonEnabled;
    }

    void Effect::SetCartoonEnabled(bool enabled)
    {
        m_cartoonEnabled = enabled;
    }

    const Vector3& Effect::GetCartoonTone() const
    {
        return m_cartoonTone.get();
    }

    OAnim3& Effect::GetCartoonToneAnim()
    {
        return m_cartoonTone;
    }

    void Effect::SetCartoonTone(const Vector3& tone)
    {
        m_cartoonTone = tone;
    }

    bool Effect::GetVignetteEnabled() const
    {
        return m_vignetteEnabled.get();
    }

    OAnimb& Effect::GetVignetteEnabledAnim()
    {
        return m_vignetteEnabled;
    }

    void Effect::SetVignetteEnabled(bool enabled)
    {
        m_vignetteEnabled = enabled;
    }

    float Effect::GetVignetteAmount() const
    {
        return m_vignetteAmount.get();
    }

    OAnimf& Effect::GetVignetteAmountAnim()
    {
        return m_vignetteAmount;
    }

    void Effect::SetVignetteAmount(float amount)
    {
        m_vignetteAmount = amount;
    }
}
