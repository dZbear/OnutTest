#pragma once
#include <onut.h>
#include "seed/Sprite.h"
#include "seed/SpriteString.h"
#include "seed/View.h"

#include <memory>

class NodeContainer;

struct NodeState
{
    enum class NodeType
    {
        Node,
        Sprite,
        SpriteString,
        Emitter,
    };

    std::string texture;
    Vector2 position;
    Vector2 scale;
    float angle;
    Color color;
    Vector2 align;
    Matrix transform;
    Matrix parentTransform;
    bool flippedH = false;
    bool flippedV = false;
    std::shared_ptr<NodeContainer> pContainer = nullptr;
    std::shared_ptr<NodeContainer> pParentContainer = nullptr;
    NodeType nodeType = NodeType::Node;
    std::vector<NodeState> bg;
    std::vector<NodeState> fg;
    OFont* pFont = nullptr;
    std::string caption;
    std::string name;

    NodeState() { }
    NodeState(const NodeState& copy);
    NodeState(std::shared_ptr<NodeContainer> in_pContainer, bool saveDeep = false);
    void apply(std::shared_ptr<NodeContainer> pParent = nullptr);
    void visit(const std::function<void(NodeState* pNodeState)>& callback);
};

class NodeContainer
{
public:
    NodeContainer() {}
    seed::Node* pNode = nullptr;
    onut::UITreeViewItem* pTreeViewItem = nullptr;
    NodeState stateOnDown;
};

inline NodeState::NodeState(const NodeState& copy)
{
    texture = copy.texture;
    position = copy.position;
    scale = copy.scale;
    angle = copy.angle;
    color = copy.color;
    align = copy.align;
    flippedH = copy.flippedH;
    flippedV = copy.flippedV;
    transform = copy.transform;
    parentTransform = copy.parentTransform;
    pContainer = copy.pContainer;
    pParentContainer = copy.pParentContainer;
    nodeType = copy.nodeType;
    bg = copy.bg;
    fg = copy.fg;
    pFont = copy.pFont;
    caption = copy.caption;
    name = copy.name;
}

extern std::unordered_map<seed::Node*, std::shared_ptr<NodeContainer>> nodesToContainers;

inline NodeState::NodeState(std::shared_ptr<NodeContainer> in_pContainer, bool saveDeep)
{
    pContainer = in_pContainer;
    nodeType = NodeType::Node;
    auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
    auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
    if (pSprite)
    {
        texture = "";
        if (pSprite->GetTexture())
        {
            texture = pSprite->GetTexture()->getName();
        }
        align = pSprite->GetAlign();
        nodeType = NodeType::Sprite;
        flippedH = pSprite->GetFlippedH();
        flippedV = pSprite->GetFlippedV();
        if (pSpriteString)
        {
            caption = pSpriteString->GetCaption();
            pFont = pSpriteString->GetFont();
            nodeType = NodeType::SpriteString;
        }
    }
    name = pContainer->pNode->GetName();
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

    if (saveDeep)
    {
        auto& bgChildren = pContainer->pNode->GetBgChildren();
        auto& fgChildren = pContainer->pNode->GetFgChildren();
        for (auto pChild : bgChildren)
        {
            auto pChildContainer = nodesToContainers[pChild];
            bg.push_back(NodeState(pChildContainer, saveDeep));
        }
        for (auto pChild : fgChildren)
        {
            auto pChildContainer = nodesToContainers[pChild];
            fg.push_back(NodeState(pChildContainer, saveDeep));
        }
    }
}

inline void NodeState::visit(const std::function<void(NodeState* pNodeState)>& callback)
{
    callback(this);
    for (auto& childState : bg)
    {
        childState.visit(callback);
    }
    for (auto& childState : fg)
    {
        childState.visit(callback);
    }
}

extern seed::View* pEditingView;

inline void NodeState::apply(std::shared_ptr<NodeContainer> pParent)
{
    if (!pContainer)
    {
        pContainer = std::make_shared<NodeContainer>();
    }
    bool forceAddTreeView = false;
    if (!pContainer->pNode)
    {
        assert(pParent);
        assert(pParent->pNode);
        switch (nodeType)
        {
            case NodeType::Node:
                pContainer->pNode = pEditingView->AddNewNode(pParent->pNode);
                break;
            case NodeType::Sprite:
                pContainer->pNode = pEditingView->AddSprite(texture, pParent->pNode);
                break;
            case NodeType::SpriteString:
            {
                std::string fontName;
                if (pFont) fontName = pFont->getName();
                pContainer->pNode = pEditingView->AddSpriteString(fontName, pParent->pNode);
                break;
            }
            default:
                assert(false);
        }
        nodesToContainers[pContainer->pNode] = pContainer;
        pContainer->pTreeViewItem = new onut::UITreeViewItem();
        pContainer->pTreeViewItem->pSharedUserData = pContainer;
        forceAddTreeView = true;
    }
    switch (nodeType)
    {
        case NodeType::Sprite:
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            assert(pSprite);
            pSprite->SetTexture(OGetTexture(texture.c_str()));
            pSprite->SetAlign(align);
            pSprite->SetFlipped(flippedH, flippedV);
            break;
        }
        case NodeType::SpriteString:
        {
            auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
            assert(pSpriteString);
            pSpriteString->SetTexture(OGetTexture(texture.c_str()));
            pSpriteString->SetAlign(align);
            pSpriteString->SetFlipped(flippedH, flippedV);
            pSpriteString->SetFont(pFont);
            pSpriteString->SetCaption(caption);
            break;
        }
    }
    pContainer->pNode->SetName(name);
    pContainer->pNode->SetPosition(position);
    pContainer->pNode->SetScale(scale);
    pContainer->pNode->SetAngle(angle);
    pContainer->pNode->SetColor(color);
    for (auto& childState : bg)
    {
        auto addTreeViewItem = (childState.pContainer && childState.pContainer->pNode && !forceAddTreeView) ? false : true;
        childState.apply(pContainer);
        if (addTreeViewItem) pContainer->pTreeViewItem->addItem(childState.pContainer->pTreeViewItem);
    }
    for (auto& childState : fg)
    {
        auto addTreeViewItem = (childState.pContainer && childState.pContainer->pNode && !forceAddTreeView) ? false : true;
        childState.apply(pContainer);
        if (addTreeViewItem) pContainer->pTreeViewItem->addItem(childState.pContainer->pTreeViewItem);
    }
}
