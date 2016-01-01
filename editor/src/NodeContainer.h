#pragma once
#include <onut.h>
#include "seed/Sprite.h"
#include "seed/View.h"

class NodeContainer;

struct NodeState
{
    enum class NodeType
    {
        Node,
        Sprite,
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
    NodeContainer* pContainer = nullptr;
    NodeType nodeType = NodeType::Node;
    std::vector<NodeState> bg;
    std::vector<NodeState> fg;

    NodeState() {}
    NodeState(const NodeState& copy);
    NodeState(NodeContainer* in_pContainer, bool saveDeep = false);
    void apply(NodeContainer* pParent = nullptr) const;
    void visit(const std::function<void(NodeState* pNodeState)>& callback);
};

class NodeContainer : public onut::Object
{
public:
    NodeContainer() { retain(); }
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
    transform = copy.transform;
    parentTransform = copy.parentTransform;
    pContainer = copy.pContainer;
    nodeType = copy.nodeType;
    bg = copy.bg;
    fg = copy.fg;
}

extern std::unordered_map<seed::Node*, NodeContainer*> nodesToContainers;

inline NodeState::NodeState(NodeContainer* in_pContainer, bool saveDeep)
{
    pContainer = in_pContainer;
    assert(pContainer);
    nodeType = NodeType::Node;
    auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
    if (pSprite)
    {
        texture = pSprite->GetTexture()->getName();
        align = pSprite->GetAlign();
        nodeType = NodeType::Sprite;
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

    if (saveDeep)
    {
        auto& bgChildren = pContainer->pNode->GetBgChildren();
        auto& fgChildren = pContainer->pNode->GetFgChildren();
        for (auto pChild : bgChildren)
        {
            auto pChildContainer = nodesToContainers[pChild];
            bg.push_back(NodeState(pChildContainer, true));
        }
        for (auto pChild : fgChildren)
        {
            auto pChildContainer = nodesToContainers[pChild];
            fg.push_back(NodeState(pChildContainer, true));
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

inline void NodeState::apply(NodeContainer* pParent) const
{
    assert(pContainer);
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
            default:
                assert(false);
        }
        nodesToContainers[pContainer->pNode] = pContainer;
        pContainer->pTreeViewItem = new onut::UITreeViewItem();
        pContainer->pTreeViewItem->pUserData = pContainer;
    }
    switch (nodeType)
    {
        case NodeType::Sprite:
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            assert(pSprite);
            pSprite->SetTexture(OGetTexture(texture.c_str()));
            pSprite->SetAlign(align);
            break;
        }
    }
    pContainer->pNode->SetPosition(position);
    pContainer->pNode->SetScale(scale);
    pContainer->pNode->SetAngle(angle);
    pContainer->pNode->SetColor(color);
    for (auto& childState : bg)
    {
        auto addTreeViewItem = childState.pContainer->pNode ? false : true;
        childState.apply(pContainer);
        if (addTreeViewItem) pContainer->pTreeViewItem->addItem(childState.pContainer->pTreeViewItem);
    }
    for (auto& childState : fg)
    {
        auto addTreeViewItem = childState.pContainer->pNode ? false : true;
        childState.apply(pContainer);
        if (addTreeViewItem) pContainer->pTreeViewItem->addItem(childState.pContainer->pTreeViewItem);
    }
}
