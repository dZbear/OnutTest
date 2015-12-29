#pragma once
#include "SeedGlobals.h"
#include "onut.h"

namespace seed
{
    class Button;
    class Sprite;
    class SpriteString;
    class Emitter;
    class View
    {
    public:

        View();
        virtual ~View();

        Sprite*         AddSprite(const string& in_textureName, Node* in_parent = nullptr, int in_zIndex = INT_MAX);
        SpriteString*   AddSpriteString(const string& in_fontName, Node* in_parent = nullptr, int in_zIndex = INT_MAX);
        Emitter*        AddEmitter(const string& in_fxName, Node* in_parent = nullptr, int in_zIndex = INT_MAX);
        void            AddNode(Node* in_node, int in_zIndex = INT_MAX);
        void            DeleteNode(Node* in_node);
        Node*           GetRootNode() { return m_rootNode; }

        Button*         AddButton(Sprite* in_sprite, const string& in_cmd);
        void            FocusButton(Button* in_button, int in_playerIndex = 1);
        Button*         GetFocusedButton(int in_playerIndex = 1);
        void            SetDefaultFocusedButton(Button* in_button, int in_playerIndex = 1);

        void            SendCommand(eAppCommand in_command, const string& in_params = "");



        // to be overriden by your "Game Specific" View
        virtual void OnUpdate() {};
        virtual void OnRender() {};
        virtual void OnShow() {};
        virtual void OnHide() {};
        virtual void OnButtonDown(Button* in_button) {};
        virtual void OnButtonUp(Button* in_button) {};
        virtual void OnButtonFocused(Button* in_button, int in_playerIndex) {};
        virtual void OnButtonFocusLost(Button* in_button, int in_playerIndex) {};
        virtual bool OnCommand(const string& in_cmd) { return false; }
        /////

        // used exclusively by the SeedApp
        void            Update();
        void            Render();
        void            Show();
        void            Hide();
        CommandVect&    GetQueuedCommands() { return m_queuedCommands; }

        // Visit all nodes and their children in order.
        // Return true from the callback to interrupt searching.
        // VisitBackward to start with last node and their last children first
        using VisitCallback = std::function<bool(Node*)>;
        void VisitNodes(const VisitCallback& callback);
        void VisitNodesBackward(const VisitCallback& callback);
        
    private:

        // root node, updating/rendering all nodes attached to it
        Node*               m_rootNode;

        // keep track of stuff in the pool
        NodeVect            m_pooledNodes;

        // sprite/node pool
        onut::Pool<true>    m_nodePool;

        // sprites with UI interractions
        ButtonVect          m_buttons;

        void            DeleteNodes();
        void            DeleteChildNodes(NodeVect& in_childVect);
        bool            IsPooled(Node* in_node);

        void            UpdateFocus();
        void            UpdateButtons();
        bool            IsInside(const Vector2& in_pos, Sprite* in_sprite);

        Button*         m_currentButton;
        Button*         m_focusedButtons[4];
        Button*         m_defaultFocusedButton[4];

        CommandVect     m_queuedCommands;

        Vector2         GetDirectionFromInputDevices(int in_playerIndex);
        Button*         GetNextFocusedButton(const Vector2& in_dir, int in_playerIndex);
        ButtonVect      GetPotentialCandidates(const Vector2& in_dir, int in_playerIndex);
        bool            AlreadyInVector(Button* in_button, ButtonVect& in_vector);
        

    };
}