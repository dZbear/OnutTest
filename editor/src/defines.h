#pragma once
#include "seed/Effect.h"
#include "seed/Emitter.h"
#include "seed/MusicEmitter.h"
#include "seed/Node.h"
#include "seed/SoundEmitter.h"
#include "seed/Sprite.h"
#include "seed/SpriteString.h"
#include "seed/TiledMapNode.h"
#include "seed/Video.h"
#include "seed/View.h"

#include "NodeContainer.h"
#include "NodeState.h"

#include <ActionManager.h>
#include <onut.h>

#include <memory>

using EffectRef = std::shared_ptr<seed::Effect>;
using EmitterRef = std::shared_ptr<seed::Emitter>;
using MusicEmitterRef = std::shared_ptr<seed::MusicEmitter>;
using NodeRef = std::shared_ptr<seed::Node>;
using SoundEmitterRef = std::shared_ptr<seed::SoundEmitter>;
using SpriteRef = std::shared_ptr<seed::Sprite>;
using SpriteStringRef = std::shared_ptr<seed::SpriteString>;
using VideoRef = std::shared_ptr<seed::Video>;
using ViewRef = std::shared_ptr<seed::View>;

enum class State
{
    Idle,
    Panning,
    Zooming,
    Moving,
    MovingHandle,
    Rotate,
    IsAboutToRotate,
    IsAboutToMove,
    IsAboutToMoveHandle,
};

enum class Handle
{
    TOP_LEFT,
    LEFT,
    BOTTOM_LEFT,
    BOTTOM,
    BOTTOM_RIGHT,
    RIGHT,
    TOP_RIGHT,
    TOP
};

struct TransformHandle
{
    Handle handle = Handle::TOP_LEFT;
    Vector2 screenPos;
    Vector2 transformDirection;
};

using TransformHandles = std::vector<TransformHandle>;
using AABB = std::vector<Vector2>;
using HandleIndex = AABB::size_type;

struct Gizmo
{
    TransformHandles transformHandles;
    AABB aabb;
};

using Zoom = float;
using ZoomIndex = int;
using Selection = std::vector<NodeContainerRef>;
using Clipboard = std::vector<NodeState>;
