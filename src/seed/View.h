#pragma once
#include "SeedGlobals.h"
#include "onut.h"

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

        Sprite*         AddSprite(const string& in_textureName, int in_zIndex = INT_MAX);
        SpriteString*   AddSpriteString(const string& in_fontName, Node* in_parent = nullptr, int in_zIndex = INT_MAX);
        void            AddNode(Node* in_node, int in_zIndex = INT_MAX);
        void            DeleteNode(Node* in_node);

        void            AddButton(Sprite* in_sprite, const string& in_cmd);
        void            SendCommand(eAppCommand in_command, const string& in_params = "");

        // to be overriden by your "Game Specific" View
        virtual void OnUpdate() {};
        virtual void OnRender() {};
        virtual void OnShow() {};
        virtual void OnHide() {};
        virtual void OnButtonDown(Button* in_button) {};
        virtual void OnButtonUp(Button* in_button) {};
        virtual void OnCommand(const string& in_cmd) {};
        /////

        // used exclusively by the SeedApp
        void            Update();
        void            Render();
        void            Show();
        void            Hide();
        CommandVect&    GetQueuedCommands() { return m_queuedCommands; }
        
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

        void            UpdateButtons();
        bool            IsInside(const Vector2& in_pos, Sprite* in_sprite);

        Button*         m_currentButton;
        Vector2         m_lastMouseDown;

        CommandVect     m_queuedCommands;
        

    };
}