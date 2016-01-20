#pragma once
#include "SeedGlobals.h"
#include "PhysicsMgr.h"
#include "onut.h"

namespace seed
{
    class Button;
    class Sprite;
    class SpriteString;
    class Emitter;
    class SoundEmitter;
    class MusicEmitter;
    class Video;
    class Effect;
    class TiledMapNode;
    class View
    {
    public:

        View();
        virtual ~View();

        Sprite*         CreateSprite(const string& in_textureName);
        Sprite*         CreateSpriteWithSpriteAnim(const string& in_animSource, const string& in_defaultAnim);
        SpriteString*   CreateSpriteString(const string& in_fontName);
        Emitter*        CreateEmitter(const string& in_fxName);
        SoundEmitter*   CreateSoundEmitter(const string& in_file);
        SoundEmitter*   CreateRandomSoundEmitter(const vector<string>& in_files);
        MusicEmitter*   CreateMusicEmitter();
        Video*          CreateVideo();
        Node*           CreateNode();
        Effect*         CreateEffect();
        TiledMapNode*   CreateTiledMapNode(const string& in_file);
        
        void            AddNode(Node* in_node, Node* in_parent = nullptr, int in_zIndex = INT_MAX);
        void            DeleteNode(Node* in_node);
        Node*           DuplicateNode(Node* in_node);
        Node*           GetRootNode() { return m_rootNode; }
        Node*           FindNode(const string& in_name);

        Button*         AddButton(Sprite* in_sprite, const string& in_cmd);
        void            FocusButton(Button* in_button, int in_playerIndex = 1);
        Button*         GetFocusedButton(int in_playerIndex = 1);
        void            SetDefaultFocusedButton(Button* in_button, int in_playerIndex = 1);

        void            SendCommand(eAppCommand in_command, const string& in_params = "");

        Vector2         GetSize() const { return m_size; }
        void            SetSize(const Vector2& in_size);

        void            Save(const string& filename);
        void            Load(const string& filename);

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

        // Physics stuff for this particular view
        PhysicsMgr&     GetPhysics();
        PhysicsBody*    CreateBoxPhysicsForNode(Node* in_node, bool in_static);
        PhysicsBody*    CreateCirclePhysicsForNode(Node* in_node, float in_radius, bool in_static);
        PhysicsBody*    GetPhysicsForNode(Node* in_node);
        
    private:

        // root node, updating/rendering all nodes attached to it
        Node*               m_rootNode;

        // keep track of stuff in the pool
        NodeVect            m_pooledNodes;

        // sprite/node pool
        onut::Pool<true>    m_nodePool;

        // sprites with UI interractions
        ButtonVect          m_buttons;

        // A view has a size that is used to scale to fit/fill
        Vector2             m_size;

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
        
        PhysicsMgr      m_physics;
        void            UpdatePhysics();
    };
}