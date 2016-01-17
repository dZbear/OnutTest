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
class PropVector3 final : public Prop<Vector3, Ttype, Tgetter, Tsetter>
{
public:
    PropVector3(onut::UITextBox* pUIX, onut::UITextBox* pUIY, onut::UITextBox* pUIZ, Tgetter getter, Tsetter setter) : Prop(getter, setter), m_pUIX(pUIX), m_pUIY(pUIY), m_pUIZ(pUIZ) {}
    void updateUI(seed::Node* pObj) override
    {
        auto objOfMyType = dynamic_cast<Ttype*>(pObj);
        if (objOfMyType)
        {
            auto val = get(objOfMyType);
            m_pUIX->setFloat(val.x);
            m_pUIY->setFloat(val.y);
            m_pUIZ->setFloat(val.z);
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
            m_pUIZ->setFloat(val.z);
        }
    }
private:
    onut::UITextBox* m_pUIX = nullptr;
    onut::UITextBox* m_pUIY = nullptr;
    onut::UITextBox* m_pUIZ = nullptr;
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

struct SpinSetting
{
    SpinSetting() {}
    SpinSetting(float inStep, float inMin = -std::numeric_limits<float>::max(), float inMax = std::numeric_limits<float>::max())
        : step(inStep), min(inMin), max(inMax)
    {
    }
    float step = 1.f;
    float min = -std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::max();
};

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerFloatProperty(const std::string& propName, const std::string& uiname, Tgetter getter, Tsetter setter, SpinSetting spinSetting = SpinSetting(), float multiplier = 1.f)
{
    auto pUI = dynamic_cast<onut::UITextBox*>(OFindUI(uiname));
    pUI->onNumberSpinStart = onStartSpinning;
    pUI->onNumberSpinEnd = onStopSpinning;
    pUI->step = spinSetting.step;
    pUI->min = spinSetting.min;
    pUI->max = spinSetting.max;
    auto prop = std::make_shared<PropFloat<Tnode, Tgetter, Tsetter>>(pUI, getter, setter, multiplier);
    props.push_back(prop);
    pUI->onTextChanged = [=](onut::UITextBox* pUI, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            prop->set(pContainer->pNode, pUI->getFloat() * multiplier);
        });
    };
}

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerDoubleProperty(const std::string& propName, const std::string& uiname, Tgetter getter, Tsetter setter, SpinSetting spinSetting = SpinSetting(), float multiplier = 1.f)
{
    auto pUI = dynamic_cast<onut::UITextBox*>(OFindUI(uiname));
    pUI->onNumberSpinStart = onStartSpinning;
    pUI->onNumberSpinEnd = onStopSpinning;
    pUI->step = spinSetting.step;
    pUI->min = spinSetting.min;
    pUI->max = spinSetting.max;
    auto prop = std::make_shared<PropDouble<Tnode, Tgetter, Tsetter>>(pUI, getter, setter, multiplier);
    props.push_back(prop);
    pUI->onTextChanged = [=](onut::UITextBox* pUI, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            prop->set(pContainer->pNode, (double)(pUI->getFloat() * multiplier));
        });
    };
}

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerVector2Property(const std::string& propName, const std::string& xName, const std::string& yName, Tgetter getter, Tsetter setter, SpinSetting spinSettingX = SpinSetting(), SpinSetting spinSettingY = SpinSetting())
{
    auto txtX = dynamic_cast<onut::UITextBox*>(OFindUI(xName));
    auto txtY = dynamic_cast<onut::UITextBox*>(OFindUI(yName));
    txtX->onNumberSpinStart = onStartSpinning;
    txtX->onNumberSpinEnd = onStopSpinning;
    txtY->onNumberSpinStart = onStartSpinning;
    txtY->onNumberSpinEnd = onStopSpinning;
    txtX->step = spinSettingX.step;
    txtX->min = spinSettingX.min;
    txtX->max = spinSettingX.max;
    txtY->step = spinSettingY.step;
    txtY->min = spinSettingY.min;
    txtY->max = spinSettingY.max;
    auto prop = std::make_shared<PropVector2<Tnode, Tgetter, Tsetter>>(txtX, txtY, getter, setter);
    props.push_back(prop);
    txtX->onTextChanged = txtY->onTextChanged = [=](onut::UITextBox* pUI, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            prop->set(pContainer->pNode, Vector2(txtX->getFloat(), txtY->getFloat()));
        });
    };
}

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerVector3Property(const std::string& propName, const std::string& xName, const std::string& yName, const std::string& zName, Tgetter getter, Tsetter setter, SpinSetting spinSettingX = SpinSetting(), SpinSetting spinSettingY = SpinSetting(), SpinSetting spinSettingZ = SpinSetting())
{
    auto txtX = dynamic_cast<onut::UITextBox*>(OFindUI(xName));
    auto txtY = dynamic_cast<onut::UITextBox*>(OFindUI(yName));
    auto txtZ = dynamic_cast<onut::UITextBox*>(OFindUI(zName));
    txtX->onNumberSpinStart = onStartSpinning;
    txtX->onNumberSpinEnd = onStopSpinning;
    txtY->onNumberSpinStart = onStartSpinning;
    txtY->onNumberSpinEnd = onStopSpinning;
    txtZ->onNumberSpinStart = onStartSpinning;
    txtZ->onNumberSpinEnd = onStopSpinning;
    txtX->step = spinSettingX.step;
    txtX->min = spinSettingX.min;
    txtX->max = spinSettingX.max;
    txtY->step = spinSettingY.step;
    txtY->min = spinSettingY.min;
    txtY->max = spinSettingY.max;
    txtZ->step = spinSettingZ.step;
    txtZ->min = spinSettingZ.min;
    txtZ->max = spinSettingZ.max;
    auto prop = std::make_shared<PropVector3<Tnode, Tgetter, Tsetter>>(txtX, txtY, txtZ, getter, setter);
    props.push_back(prop);
    txtX->onTextChanged = txtY->onTextChanged = txtZ->onTextChanged = [=](onut::UITextBox* pUI, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            prop->set(pContainer->pNode, Vector3(txtX->getFloat(), txtY->getFloat(), txtZ->getFloat()));
        });
    };
}

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerColorProperty(const std::string& propName, const std::string& uiname, const std::string& alphaName, Tgetter getter, Tsetter setter)
{
    auto txtAlpha = dynamic_cast<onut::UITextBox*>(OFindUI(alphaName));
    auto pUI = dynamic_cast<onut::UIPanel*>(OFindUI(uiname));
    txtAlpha->onNumberSpinStart = onStartSpinning;
    txtAlpha->onNumberSpinEnd = onStopSpinning;
    txtAlpha->min = 0.f;
    txtAlpha->max = 100.f;
    auto prop = std::make_shared<PropColor<Tnode, Tgetter, Tsetter>>(pUI, txtAlpha, getter, setter);
    props.push_back(prop);
    pUI->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& evt)
    {
        static COLORREF g_acrCustClr[16]; // array of custom colors

        CHOOSECOLOR colorChooser = {0};
        DWORD rgbCurrent; // initial color selection
        rgbCurrent = (DWORD)pUI->color.packed;
        rgbCurrent = ((rgbCurrent >> 24) & 0x000000ff) | ((rgbCurrent >> 8) & 0x0000ff00) | ((rgbCurrent << 8) & 0x00ff0000);
        colorChooser.lStructSize = sizeof(colorChooser);
        colorChooser.hwndOwner = OWindow->getHandle();
        colorChooser.lpCustColors = (LPDWORD)g_acrCustClr;
        colorChooser.rgbResult = rgbCurrent;
        colorChooser.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColor(&colorChooser) == TRUE)
        {
            onut::sUIColor color;
            rgbCurrent = colorChooser.rgbResult;
            color.packed = ((rgbCurrent << 24) & 0xff000000) | ((rgbCurrent << 8) & 0x00ff0000) | ((rgbCurrent >> 8) & 0x0000ff00) | 0x000000ff;
            color.unpack();
            pUI->color = color;
            changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
            {
                auto colorBefore = pContainer->pNode->GetColor();
                prop->set(pContainer->pNode, Color(color.r, color.g, color.b, colorBefore.w));
            });
        }
    };
    txtAlpha->onTextChanged = [=](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            auto alpha = txtAlpha->getFloat() / 100.f;
            auto colorBefore = pContainer->pNode->GetColor();
            prop->set(pContainer->pNode, Color(colorBefore, alpha));
        });
    };
}

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerStringProperty(const std::string& propName, const std::string& uiname, Tgetter getter, Tsetter setter)
{
    auto pUI = dynamic_cast<onut::UITextBox*>(OFindUI(uiname));
    auto prop = std::make_shared<PropString<Tnode, Tgetter, Tsetter>>(pUI, getter, setter);
    props.push_back(prop);
    pUI->onTextChanged = [=](onut::UITextBox* pUI, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            prop->set(pContainer->pNode, pUI->textComponent.text);
        });
    };
}

template<typename Tnode, typename Tgetter, typename Tsetter>
void registerBoolProperty(const std::string& propName, const std::string& uiname, Tgetter getter, Tsetter setter)
{
    auto pUI = dynamic_cast<onut::UICheckBox*>(OFindUI(uiname));
    auto prop = std::make_shared<PropBool<Tnode, Tgetter, Tsetter>>(pUI, getter, setter);
    props.push_back(prop);
    pUI->onCheckChanged = [=](onut::UICheckBox* pUI, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change " + propName, [=](std::shared_ptr<NodeContainer> pContainer)
        {
            prop->set(pContainer->pNode, pUI->getIsChecked());
        });
    };
}
