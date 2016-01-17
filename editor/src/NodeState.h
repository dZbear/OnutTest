#pragma once
#include "defines.h"

class NodeState;
using NodeStateRef = std::shared_ptr<NodeState>;

class NodeState : public std::enable_shared_from_this<NodeState>
{
public:
    enum class NodeType
    {
        Node,
        Sprite,
        SpriteString,
        Emitter,
        SoundEmitter,
        MusicEmitter,
        Video,
        Effect,
        TiledMapNode,
    };

    struct Node
    {
        std::string name;
        Vector2 position;
        Vector2 scale;
        float angle;
        Color color;
        Matrix transform;
        Matrix parentTransform;
        std::vector<NodeState> bg;
        std::vector<NodeState> fg;
    };
    struct Sprite
    {
        std::string texture;
        Vector2 align;
        bool flippedH;
        bool flippedV;
        onut::SpriteBatch::eBlendMode blend;
        onut::SpriteBatch::eFiltering filtering;
    };
    struct StringSprite
    {
        std::string caption;
        OFont* pFont;
    };
    struct Emitter
    {
        std::string fxName;
        onut::SpriteBatch::eBlendMode blend;
        onut::SpriteBatch::eFiltering filtering;
        bool emitWorld;
    };
    struct SoundEmitter
    {
        std::string source;
        bool loop;
        float volume;
        float balance;
        float pitch;
        bool positionBased;
    };
    struct MusicEmitter
    {
        std::string source;
        bool loop;
        float volume;
    };
    struct Video
    {
        std::string source;
        bool loop;
        float volume;
        Vector2 dimensions;
        double playRate;
    };
    struct Effect
    {
        bool blurEnabled;
        float blurAmount;
        bool sepiaEnabled;
        Vector3 sepiaTone;
        float sepiaSaturation;
        float sepiaAmount;
        bool crtEnabled;
        bool cartoonEnabled;
        Vector3 cartoonTone;
        bool vignetteEnabled;
        float vignetteAmount;
    };
    struct TiledMapNode
    {
        std::string file;
    };

    NodeType nodeType = NodeType::Node;

    Node node;
    Sprite sprite;
    StringSprite stringSprite;
    Emitter emitter;
    SoundEmitter soundEmitter;
    MusicEmitter musicEmitter;
    Video video;
    Effect effect;
    TiledMapNode tiledMapNode;

    NodeContainerRef pContainer = nullptr;
    NodeContainerRef pParentContainer = nullptr;

    NodeState() {}
    NodeState(NodeStateRef copy);
    NodeState(NodeContainerRef in_pContainer, bool saveDeep = false);
    void apply(NodeContainerRef pParentContainer = nullptr);
    void visit(const std::function<void(NodeStateRef nodeState)>& callback);
};
