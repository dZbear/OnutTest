#include "NodeState.h"

NodeState::NodeState(NodeStateRef copy)
{
    nodeType = copy->nodeType;
    node = copy->node;
    switch (copy->nodeType)
    {
        case NodeType::SpriteString: stringSprite = copy->stringSprite; // Intentional fall through
        case NodeType::Sprite: sprite = copy->sprite; break;
        case NodeType::Emitter: emitter = copy->emitter; break;
        case NodeType::SoundEmitter: soundEmitter = copy->soundEmitter; break;
        case NodeType::MusicEmitter: musicEmitter = copy->musicEmitter; break;
        case NodeType::Video: video = copy->video; break;
        case NodeType::Effect: effect = copy->effect; break;
        case NodeType::TiledMapNode: tiledMapNode = copy->tiledMapNode; break;
    }
    pContainer = copy->pContainer;
    pParentContainer = copy->pParentContainer;
}

extern std::unordered_map<seed::Node*, NodeContainerRef> nodesToContainers;

NodeState::NodeState(std::shared_ptr<NodeContainer> in_pContainer, bool saveDeep)
{
    pContainer = in_pContainer;

    auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
    auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
    auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
    auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
    auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
    auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
    auto pEffect = dynamic_cast<seed::Effect*>(pContainer->pNode);
    auto pTiledMapNode = dynamic_cast<seed::TiledMapNode*>(pContainer->pNode);

    nodeType = NodeType::Node;
    node.name = pContainer->pNode->GetName();
    node.position = pContainer->pNode->GetPosition();
    node.scale = pContainer->pNode->GetScale();
    node.angle = pContainer->pNode->GetAngle();
    node.color = pContainer->pNode->GetColor();
    node.transform = pContainer->pNode->GetTransform();
    node.parentTransform = Matrix::Identity;
    if (pContainer->pNode->GetParent())
    {
        node.parentTransform = pContainer->pNode->GetParent()->GetTransform();
    }

    if (pSprite)
    {
        nodeType = NodeType::Sprite;
        sprite.texture = "";
        if (pSprite->GetTexture())
        {
            sprite.texture = pSprite->GetTexture()->getName();
        }
        sprite.align = pSprite->GetAlign();
        sprite.flippedH = pSprite->GetFlippedH();
        sprite.flippedV = pSprite->GetFlippedV();
        sprite.blend = pSprite->GetBlend();
        sprite.filtering = pSprite->GetFilter();
        if (pSpriteString)
        {
            nodeType = NodeType::SpriteString;
            stringSprite.caption = pSpriteString->GetCaption();
            stringSprite.pFont = pSpriteString->GetFont();
        }
    }
    else if (pEmitter)
    {
        nodeType = NodeType::Emitter;
        emitter.fxName = pEmitter->GetFxName();
        emitter.blend = pEmitter->GetBlend();
        emitter.filtering = pEmitter->GetFilter();
        emitter.emitWorld = pEmitter->GetEmitWorld();
    }
    else if (pSoundEmitter)
    {
        nodeType = NodeType::SoundEmitter;
        soundEmitter.source = pSoundEmitter->GetSource();
        soundEmitter.loop = pSoundEmitter->GetLoops();
        soundEmitter.volume = pSoundEmitter->GetVolume();
        soundEmitter.balance = pSoundEmitter->GetBalance();
        soundEmitter.pitch = pSoundEmitter->GetPitch();
        soundEmitter.positionBased = pSoundEmitter->GetPositionBased();
    }
    else if (pMusicEmitter)
    {
        nodeType = NodeType::MusicEmitter;
        musicEmitter.source = pMusicEmitter->GetSource();
        musicEmitter.loop = pMusicEmitter->GetLoops();
        musicEmitter.volume = pMusicEmitter->GetVolume();
    }
    else if (pVideo)
    {
        nodeType = NodeType::Video;
        video.source = pVideo->GetSource();
        video.loop = pVideo->GetLoops();
        video.volume = pVideo->GetVolume();
        video.dimensions = pVideo->GetDimensions();
        video.playRate = pVideo->GetPlayRate();
    }
    else if (pEffect)
    {
        nodeType = NodeType::Effect;
        effect.blurEnabled = pEffect->GetBlurEnabled();
        effect.blurAmount = pEffect->GetBlurAmount();
        effect.sepiaEnabled = pEffect->GetSepiaEnabled();
        effect.sepiaTone = pEffect->GetSepiaTone();
        effect.sepiaSaturation = pEffect->GetSepiaSaturation();
        effect.sepiaAmount = pEffect->GetSepiaAmount();
        effect.crtEnabled = pEffect->GetCrtEnabled();
        effect.cartoonEnabled = pEffect->GetCartoonEnabled();
        effect.cartoonTone = pEffect->GetCartoonTone();
        effect.vignetteEnabled = pEffect->GetVignetteEnabled();
        effect.vignetteAmount = pEffect->GetVignetteAmount();
    }
    else if (pTiledMapNode)
    {
        nodeType = NodeType::TiledMapNode;
        tiledMapNode.file = pTiledMapNode->GetFile();
    }
    if (saveDeep)
    {
        auto& bgChildren = pContainer->pNode->GetBgChildren();
        auto& fgChildren = pContainer->pNode->GetFgChildren();
        for (auto pChild : bgChildren)
        {
            auto pChildContainer = nodesToContainers[pChild];
            node.bg.push_back(NodeState(pChildContainer, saveDeep));
        }
        for (auto pChild : fgChildren)
        {
            auto pChildContainer = nodesToContainers[pChild];
            node.fg.push_back(NodeState(pChildContainer, saveDeep));
        }
    }
}

void NodeState::visit(const std::function<void(NodeStateRef pNodeState)>& callback)
{
    callback(shared_from_this());
    for (auto& childState : node.bg)
    {
        childState.visit(callback);
    }
    for (auto& childState : node.fg)
    {
        childState.visit(callback);
    }
}

extern seed::View* pEditingView;

void NodeState::apply(std::shared_ptr<NodeContainer> pParent)
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
                pContainer->pNode = pEditingView->CreateSprite(sprite.texture);
                break;
            case NodeType::SpriteString:
            {
                std::string fontName;
                if (stringSprite.pFont) fontName = stringSprite.pFont->getName();
                pContainer->pNode = pEditingView->CreateSpriteString(fontName);
                break;
            }
            case NodeType::Emitter:
                pContainer->pNode = pEditingView->CreateEmitter(emitter.fxName);
                break;
            case NodeType::SoundEmitter:
                pContainer->pNode = pEditingView->CreateSoundEmitter(soundEmitter.source);
                break;
            case NodeType::MusicEmitter:
                pContainer->pNode = pEditingView->CreateMusicEmitter();
                break;
            case NodeType::Video:
                pContainer->pNode = pEditingView->CreateVideo();
                break;
            case NodeType::Effect:
                pContainer->pNode = pEditingView->CreateEffect();
                break;
            case NodeType::TiledMapNode:
                pContainer->pNode = pEditingView->CreateTiledMapNode(tiledMapNode.file);
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
        case NodeType::SpriteString:
        {
            auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
            assert(pSpriteString);
            pSpriteString->SetFont(stringSprite.pFont);
            pSpriteString->SetCaption(stringSprite.caption);
        } // Intentional fall through (Bad.. I know)
        case NodeType::Sprite:
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            assert(pSprite);
            pSprite->SetTexture(OGetTexture(sprite.texture.c_str()));
            pSprite->SetAlign(sprite.align);
            pSprite->SetFlipped(sprite.flippedH, sprite.flippedV);
            pSprite->SetBlend(sprite.blend);
            pSprite->SetFilter(sprite.filtering);
            break;
        }
        case NodeType::Emitter:
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            assert(pEmitter);
            pEmitter->Init(emitter.fxName);
            pEmitter->SetBlend(emitter.blend);
            pEmitter->SetFilter(emitter.filtering);
            pEmitter->SetEmitWorld(emitter.emitWorld);
            break;
        }
        case NodeType::SoundEmitter:
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            assert(pSoundEmitter);
            if (pSoundEmitter->GetSource() != soundEmitter.source)
            {
                pSoundEmitter->Init(soundEmitter.source);
            }
            pSoundEmitter->SetVolume(soundEmitter.volume);
            pSoundEmitter->SetBalance(soundEmitter.balance);
            pSoundEmitter->SetPitch(soundEmitter.pitch);
            pSoundEmitter->SetLoops(soundEmitter.loop);
            pSoundEmitter->SetPositionBased(soundEmitter.positionBased);
            break;
        }
        case NodeType::MusicEmitter:
        {
            auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
            assert(pMusicEmitter);
            pMusicEmitter->SetSource(musicEmitter.source);
            pMusicEmitter->SetVolume(musicEmitter.volume);
            pMusicEmitter->SetLoops(musicEmitter.loop);
            break;
        }
        case NodeType::Video:
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            assert(pVideo);
            pVideo->SetSource(video.source);
            pVideo->SetVolume(video.volume);
            pVideo->SetLoops(video.loop);
            pVideo->SetDimensions(video.dimensions);
            pVideo->SetPlayRate(video.playRate);
            break;
        }
        case NodeType::Effect:
        {
            auto pEffect = dynamic_cast<seed::Effect*>(pContainer->pNode);
            assert(pEffect);
            pEffect->SetBlurEnabled(effect.blurEnabled);
            pEffect->SetBlurAmount(effect.blurAmount);
            pEffect->SetSepiaEnabled(effect.sepiaEnabled);
            pEffect->SetSepiaTone(effect.sepiaTone);
            pEffect->SetSepiaSaturation(effect.sepiaSaturation);
            pEffect->SetSepiaAmount(effect.sepiaAmount);
            pEffect->SetCrtEnabled(effect.crtEnabled);
            pEffect->SetCartoonEnabled(effect.cartoonEnabled);
            pEffect->SetCartoonTone(effect.cartoonTone);
            pEffect->SetVignetteEnabled(effect.vignetteEnabled);
            pEffect->SetVignetteAmount(effect.vignetteAmount);
            break;
        }
        case NodeType::TiledMapNode:
        {
            auto pTiledMapNode = dynamic_cast<seed::TiledMapNode*>(pContainer->pNode);
            assert(pTiledMapNode);
            pTiledMapNode->Init(tiledMapNode.file);
            break;
        }
    }
    pContainer->pNode->SetName(node.name);
    pContainer->pNode->SetPosition(node.position);
    pContainer->pNode->SetScale(node.scale);
    pContainer->pNode->SetAngle(node.angle);
    pContainer->pNode->SetColor(node.color);
    for (auto& childState : node.bg)
    {
        auto addTreeViewItem = (childState.pContainer && childState.pContainer->pNode && !forceAddTreeView) ? false : true;
        childState.apply(pContainer);
        if (addTreeViewItem) pContainer->pTreeViewItem->addItem(childState.pContainer->pTreeViewItem);
    }
    for (auto& childState : node.fg)
    {
        auto addTreeViewItem = (childState.pContainer && childState.pContainer->pNode && !forceAddTreeView) ? false : true;
        childState.apply(pContainer);
        if (addTreeViewItem) pContainer->pTreeViewItem->addItem(childState.pContainer->pTreeViewItem);
    }
}
