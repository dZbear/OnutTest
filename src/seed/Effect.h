#pragma once

#include "Node.h"

namespace seed
{
    class Effect : public Node
    {
    public:
        Effect();
        virtual ~Effect();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*                   Duplicate() const;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        bool            GetBlurEnabled() const;
        OAnimb&         GetBlurEnabledAnim();
        void            SetBlurEnabled(bool enabled);
        float           GetBlurAmount() const;
        OAnimf&         GetBlurAmountAnim();
        void            SetBlurAmount(float amount);

        bool            GetSepiaEnabled() const;
        OAnimb&         GetSepiaEnabledAnim();
        void            SetSepiaEnabled(bool enabled);
        const Vector3&  GetSepiaTone() const;
        OAnim3&         GetSepiaToneAnim();
        void            SetSepiaTone(const Vector3& tone);
        float           GetSepiaSaturation() const;
        OAnimf&         GetSepiaSaturationAnim();
        void            SetSepiaSaturation(float saturation);
        float           GetSepiaAmount() const;
        OAnimf&         GetSepiaAmountAnim();
        void            SetSepiaAmount(float amount);

        bool            GetCrtEnabled() const;
        OAnimb&         GetCrtEnabledAnim();
        void            SetCrtEnabled(bool enabled);

        bool            GetCartoonEnabled() const;
        OAnimb&         GetCartoonEnabledAnim();
        void            SetCartoonEnabled(bool enabled);
        const Vector3&  GetCartoonTone() const;
        OAnim3&         GetCartoonToneAnim();
        void            SetCartoonTone(const Vector3& tone);

        bool            GetVignetteEnabled() const;
        OAnimb&         GetVignetteEnabledAnim();
        void            SetVignetteEnabled(bool enabled);
        float           GetVignetteAmount() const;
        OAnimf&         GetVignetteAmountAnim();
        void            SetVignetteAmount(float amount);

        // only to be used by the seed sdk
        virtual void    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;

    protected:
        virtual void    Copy(Node* in_copy) const;

        OTexture*       m_effectTarget;

        // Blur
        OAnimb          m_blurEnabled = false;
        OAnimf          m_blurAmount = 16.f;

        // Sepia
        OAnimb          m_sepiaEnabled = false;
        OAnim3          m_sepiaTone = Vector3(1.40f, 1.10f, 0.90f);
        OAnimf          m_sepiaSaturation = 0.f;
        OAnimf          m_sepiaAmount = .75f;

        // CRT
        OAnimb          m_crtEnabled = false;
        
        // Cartoon
        OAnimb          m_cartoonEnabled = false;
        OAnim3          m_cartoonTone = Vector3(2, 5, 1);

        // Vignette
        OAnimb          m_vignetteEnabled = false;
        OAnimf          m_vignetteAmount = .5f;
    };
}
