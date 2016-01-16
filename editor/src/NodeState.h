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
    };

    std::string name;
    Vector2 position;
    Vector2 scale;
    Color color;
    Matrix transform;
    Matrix parentTransform;
    std::vector<NodeState> bg;
    std::vector<NodeState> fg;

    std::string source;
    Vector2 align;
    float angle;
    bool flippedH = false;
    bool flippedV = false;
    NodeContainerRef pContainer = nullptr;
    NodeContainerRef pParentContainer = nullptr;
    NodeType nodeType = NodeType::Node;
    OFont* pFont = nullptr;
    std::string caption;
    onut::SpriteBatch::eBlendMode blend;
    onut::SpriteBatch::eFiltering filtering;
    bool emitWorld = true;
    bool loop = false;
    float volume = 1.f;
    float balance = 0.f;
    float pitch = 1.f;
    Vector2 dimensions;

    NodeState() {}
    NodeState(NodeStateRef copy);
    NodeState(NodeContainerRef in_pContainer, bool saveDeep = false);
    void apply(NodeContainerRef pParentContainer = nullptr);
    void visit(const std::function<void(NodeStateRef nodeState)>& callback);
};
