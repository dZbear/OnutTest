#pragma once
#include <onut.h>
#include "seed/Sprite.h"

class NodeContainer;

struct SpriteState
{
    std::string texture;
    Vector2 position;
    Vector2 scale;
    float angle;
    Color color;
    Vector2 align;
    Matrix transform;
    Matrix parentTransform;
    NodeContainer* pContainer = nullptr;

    SpriteState() {}
    SpriteState(const SpriteState& copy);
    SpriteState(NodeContainer* in_pContainer);
    void apply() const;
};

class NodeContainer : public onut::Object
{
public:
    NodeContainer() { retain(); }
    seed::Node* pNode = nullptr;
    onut::UITreeViewItem* pTreeViewItem = nullptr;
    SpriteState stateOnDown;
};

inline SpriteState::SpriteState(const SpriteState& copy)
{
    texture = copy.texture;
    position = copy.position;
    scale = copy.scale;
    angle = copy.angle;
    color = copy.color;
    align = copy.align;
    transform = copy.transform;
    parentTransform = copy.parentTransform;
    pContainer = copy.pContainer;
}

inline SpriteState::SpriteState(NodeContainer* in_pContainer)
{
    pContainer = in_pContainer;
    assert(pContainer);
    auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
    if (pSprite)
    {
        texture = pSprite->GetTexture()->getName();
        align = pSprite->GetAlign();
    }
    position = pContainer->pNode->GetPosition();
    scale = pContainer->pNode->GetScale();
    angle = pContainer->pNode->GetAngle();
    color = pContainer->pNode->GetColor();
    transform = pContainer->pNode->GetTransform();
    parentTransform = Matrix::Identity;
    if (pContainer->pNode->GetParent())
    {
        parentTransform = pContainer->pNode->GetParent()->GetTransform();
    }
}

inline void SpriteState::apply() const
{
    assert(pContainer);
    auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
    if (pSprite)
    {
        pSprite->SetTexture(OGetTexture(texture.c_str()));
        pSprite->SetAlign(align);
    }
    pContainer->pNode->SetPosition(position);
    pContainer->pNode->SetScale(scale);
    pContainer->pNode->SetAngle(angle);
    pContainer->pNode->SetColor(color);
}
