#pragma once
#include "SeedGlobals.h"
#include "onut.h"

#include <limits>

namespace seed
{
    class Button;
    class Sprite;
    class SpriteString;
    class View
    {
    public:

        View();
        virtual ~View();

        Sprite*            AddSprite(const string& in_textureName, int in_zIndex = std::numeric_limits<int>::max());
        SpriteString*   AddSpriteString(const string& in_fontName, Node* in_parent = nullptr, int in_zIndex = std::numeric_limits<int>::max());
        void            AddNode(Node* in_node, int in_zIndex = std::numeric_limits<int>::max());
        void            DeleteNode(Node* in_node);

        void            AddButton(Sprite* in_sprite, const string& in_cmd);

        // to be overriden by your "Game Specific" View
        virtual void OnUpdate() {};
        virtual void OnRender() {};
        virtual void OnShow() {};
        virtual void OnHide() {};
        /////

        // used exclusively by the SeedApp
        void Update();
        void Render();
        void Show();
        void Hide();
        
    private:

        // actual sprites/nodes updated/rendered by this view
        NodeVect            m_nodes;

        // keep track of stuff in the pool
        NodeVect            m_pooledNodes;

        // sprite/node pool
        onut::Pool<true>    m_nodePool;

        // sprites with UI interractions
        ButtonVect          m_buttons;

        void            InsertNode(Node* in_node, int in_zIndex);
        void            DeleteNodes();
        bool            IsPooled(Node* in_node);


    };
}