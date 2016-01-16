#pragma once
#include "seed/Node.h"

#include <UI.h>
#include <memory>

class NodeState;

class NodeContainer
{
public:
    seed::Node* pNode = nullptr;
    onut::UITreeViewItem* pTreeViewItem = nullptr;
    shared_ptr<NodeState> stateOnDown;
};

using NodeContainerRef = std::shared_ptr<NodeContainer>;
