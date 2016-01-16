#pragma once
#include "seed/View.h"
#include "seed/Node.h"

#include <onut.h>

class IProp
{
public:
    virtual void updateUI(seed::Node* pObj) = 0;
    virtual void updateUI(seed::View* pView) = 0;
};
template<typename TpropType, typename Ttype, typename Tgetter, typename Tsetter>class Prop : public IProp
{
public:
    Prop(Tgetter getter, Tsetter setter) : m_getter(getter), m_setter(setter) {}
    void set(seed::Node* pObj, TpropType val)
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto setter = std::bind(m_setter, objOfMyType, std::placeholders::_1);
            setter(val);
        }
    }
protected:
    TpropType get(Ttype* pObj) const
    {
        auto getter = std::bind(m_getter, pObj);
        return getter();
    }
    Tgetter m_getter;
    Tsetter m_setter;
};

template<typename Ttype, typename Tgetter, typename Tsetter>
class PropFloat final : public Prop<float, Ttype, Tgetter, Tsetter>
{
public:
    PropFloat(onut::UITextBox* pUI, Tgetter getter, Tsetter setter, float multiplier = 1.f) : Prop(getter, setter), m_pUI(pUI), m_multiplier(multiplier) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUI->setFloat(val / m_multiplier);
        }
    }
    void updateUI(seed::View* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUI->setFloat(val / m_multiplier);
        }
    }
private:
    onut::UITextBox* m_pUI = nullptr;
    float m_multiplier = 1.f;
};

template<typename Ttype, typename Tgetter, typename Tsetter>
class PropDouble final : public Prop<double, Ttype, Tgetter, Tsetter>
{
public:
    PropDouble(onut::UITextBox* pUI, Tgetter getter, Tsetter setter, float multiplier = 1.f) : Prop(getter, setter), m_pUI(pUI), m_multiplier(multiplier) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = (float)get(objOfMyType);
            m_pUI->setFloat(val / m_multiplier);
        }
    }
    void updateUI(seed::View* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = (float)get(objOfMyType);
            m_pUI->setFloat(val / m_multiplier);
        }
    }
private:
    onut::UITextBox* m_pUI = nullptr;
    float m_multiplier = 1.f;
};

template<typename Ttype, typename Tgetter, typename Tsetter>
class PropVector2 final : public Prop<Vector2, Ttype, Tgetter, Tsetter>
{
public:
    PropVector2(onut::UITextBox* pUIX, onut::UITextBox* pUIY, Tgetter getter, Tsetter setter) : Prop(getter, setter), m_pUIX(pUIX), m_pUIY(pUIY) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUIX->setFloat(val.x);
            m_pUIY->setFloat(val.y);
        }
    }
    void updateUI(seed::View* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUIX->setFloat(val.x);
            m_pUIY->setFloat(val.y);
        }
    }
private:
    onut::UITextBox* m_pUIX = nullptr;
    onut::UITextBox* m_pUIY = nullptr;
};

template<typename Ttype, typename Tgetter, typename Tsetter>
class PropColor final : public Prop<Color, Ttype, Tgetter, Tsetter>
{
public:
    PropColor(onut::UIPanel* pUIColor, onut::UITextBox* pUIAlpha, Tgetter getter, Tsetter setter) : Prop(getter, setter), m_pUIColor(pUIColor), m_pUIAlpha(pUIAlpha) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUIColor->color.r = val.x;
            m_pUIColor->color.g = val.y;
            m_pUIColor->color.b = val.z;
            m_pUIColor->color.pack();
            m_pUIAlpha->setFloat(val.w * 100.f);
        }
    }
    void updateUI(seed::View* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUIColor->color.r = val.x;
            m_pUIColor->color.g = val.y;
            m_pUIColor->color.b = val.z;
            m_pUIColor->color.pack();
            m_pUIAlpha->setFloat(val.w * 100.f);
        }
    }
private:
    onut::UIPanel* m_pUIColor = nullptr;
    onut::UITextBox* m_pUIAlpha = nullptr;
};

template<typename Ttype, typename Tgetter, typename Tsetter>
class PropString final : public Prop<std::string, Ttype, Tgetter, Tsetter>
{
public:
    PropString(onut::UITextBox* pUI, Tgetter getter, Tsetter setter) : Prop(getter, setter), m_pUI(pUI) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUI->textComponent.text = val;
        }
    }
    void updateUI(seed::View* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUI->textComponent.text = val;
        }
    }
private:
    onut::UITextBox* m_pUI = nullptr;
};

template<typename Ttype, typename Tgetter, typename Tsetter>
class PropBool final : public Prop<bool, Ttype, Tgetter, Tsetter>
{
public:
    PropBool(onut::UICheckBox* pUI, Tgetter getter, Tsetter setter) : Prop(getter, setter), m_pUI(pUI) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUI->setIsChecked(val);
        }
    }
    void updateUI(seed::View* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUI->setIsChecked(val);
        }
    }
private:
    onut::UICheckBox* m_pUI = nullptr;
};
