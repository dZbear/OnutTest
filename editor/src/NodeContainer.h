#pragma once
#include <onut.h>
#include "seed/Emitter.h"
#include "seed/MusicEmitter.h"
#include "seed/SoundEmitter.h"
#include "seed/Sprite.h"
#include "seed/SpriteString.h"
#include "seed/Video.h"
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
        SoundEmitter,
        MusicEmitter,
        Video,
    };

    std::string source;
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
    onut::SpriteBatch::eBlendMode blend;
    onut::SpriteBatch::eFiltering filtering;
    bool emitWorld = true;
    bool loop = false;
    float volume = 1.f;
    float balance = 0.f;
    float pitch = 1.f;
    Vector2 dimensions;

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
    source = copy.source;
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
    blend = copy.blend;
    filtering = copy.filtering;
    emitWorld = copy.emitWorld;
    loop = copy.loop;
    volume = copy.volume;
    balance = copy.balance;
    pitch = copy.pitch;
    dimensions = copy.dimensions;
}

extern std::unordered_map<seed::Node*, std::shared_ptr<NodeContainer>> nodesToContainers;

inline NodeState::NodeState(std::shared_ptr<NodeContainer> in_pContainer, bool saveDeep)
{
    pContainer = in_pContainer;
    nodeType = NodeType::Node;
    auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
    auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
    auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
    auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
    auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
    auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
    if (pSprite)
    {
        source = "";
        if (pSprite->GetTexture())
        {
            source = pSprite->GetTexture()->getName();
        }
        align = pSprite->GetAlign();
        nodeType = NodeType::Sprite;
        flippedH = pSprite->GetFlippedH();
        flippedV = pSprite->GetFlippedV();
        blend = pSprite->GetBlend();
        filtering = pSprite->GetFilter();
        if (pSpriteString)
        {
            caption = pSpriteString->GetCaption();
            pFont = pSpriteString->GetFont();
            nodeType = NodeType::SpriteString;
        }
    }
    else if (pEmitter)
    {
        source = pEmitter->GetFxName();
        blend = pEmitter->GetBlend();
        filtering = pEmitter->GetFilter();
        emitWorld = pEmitter->GetEmitWorld();
        nodeType = NodeType::Emitter;
    }
    else if (pSoundEmitter)
    {
        source = pSoundEmitter->GetSource();
        loop = pSoundEmitter->GetLoops();
        volume = pSoundEmitter->GetVolume();
        balance = pSoundEmitter->GetBalance();
        pitch = pSoundEmitter->GetPitch();
        emitWorld = pSoundEmitter->GetPositionBasedBalance() || pSoundEmitter->GetPositionBasedVolume();
        nodeType = NodeType::SoundEmitter;
    }
    else if (pMusicEmitter)
    {
        source = pMusicEmitter->GetSource();
        loop = pMusicEmitter->GetLoops();
        volume = pMusicEmitter->GetVolume();
        nodeType = NodeType::MusicEmitter;
    }
    else if (pVideo)
    {
        source = pVideo->GetSource();
        loop = pVideo->GetLoops();
        volume = pVideo->GetVolume();
        dimensions = pVideo->GetDimensions();
        pitch = (float)pVideo->GetPlayRate();
        nodeType = NodeType::Video;
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
                pContainer->pNode = pEditingView->CreateNode();
                break;
            case NodeType::Sprite:
                pContainer->pNode = pEditingView->CreateSprite(source);
                break;
            case NodeType::SpriteString:
            {
                std::string fontName;
                if (pFont) fontName = pFont->getName();
                pContainer->pNode = pEditingView->CreateSpriteString(fontName);
                break;
            }
            case NodeType::Emitter:
                pContainer->pNode = pEditingView->CreateEmitter(source);
                break;
            case NodeType::SoundEmitter:
                pContainer->pNode = pEditingView->CreateSoundEmitter(source);
                break;
            case NodeType::MusicEmitter:
                pContainer->pNode = pEditingView->CreateMusicEmitter();
                break;
            case NodeType::Video:
                pContainer->pNode = pEditingView->CreateVideo();
                break;
            default:
                assert(false);
        }
        pEditingView->AddNode(pContainer->pNode, pParent->pNode);
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
            pSprite->SetTexture(OGetTexture(source.c_str()));
            pSprite->SetAlign(align);
            pSprite->SetFlipped(flippedH, flippedV);
            pSprite->SetBlend(blend);
            pSprite->SetFilter(filtering);
            break;
        }
        case NodeType::SpriteString:
        {
            auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
            assert(pSpriteString);
            pSpriteString->SetTexture(OGetTexture(source.c_str()));
            pSpriteString->SetAlign(align);
            pSpriteString->SetFlipped(flippedH, flippedV);
            pSpriteString->SetFont(pFont);
            pSpriteString->SetCaption(caption);
            pSpriteString->SetBlend(blend);
            pSpriteString->SetFilter(filtering);
            break;
        }
        case NodeType::Emitter:
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            assert(pEmitter);
            pEmitter->Init(source);
            pEmitter->SetBlend(blend);
            pEmitter->SetFilter(filtering);
            pEmitter->SetEmitWorld(emitWorld);
            break;
        }
        case NodeType::SoundEmitter:
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            assert(pSoundEmitter);
            if (pSoundEmitter->GetSource() != source)
            {
                pSoundEmitter->Init(source);
            }
            pSoundEmitter->SetVolume(volume);
            pSoundEmitter->SetBalance(balance);
            pSoundEmitter->SetPitch(pitch);
            pSoundEmitter->SetLoops(loop);
            pSoundEmitter->SetPositionBasedVolume(emitWorld);
            pSoundEmitter->SetPositionBasedBalance(emitWorld);
            break;
        }
        case NodeType::MusicEmitter:
        {
            auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
            assert(pMusicEmitter);
            pMusicEmitter->SetSource(source);
            pMusicEmitter->SetVolume(volume);
            pMusicEmitter->SetLoops(loop);
            break;
        }
        case NodeType::Video:
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            assert(pVideo);
            pVideo->SetSource(source);
            pVideo->SetVolume(volume);
            pVideo->SetLoops(loop);
            pVideo->SetDimensions(dimensions);
            pVideo->SetPlayRate(pitch);
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
