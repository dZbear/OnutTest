#include "View.h"
#include "Sprite.h"
#include "SpriteString.h"
#include "Emitter.h"
#include "SoundEmitter.h"
#include "MusicEmitter.h"
#include "Video.h"
#include "Effect.h"
#include "Button.h"
#include "onut.h"
#include "tinyxml2.h"
#include "TiledMapNode.h"
#include "PhysicsBody.h"

#define VIEW_DEFAULT_NODE_COUNT 1024

namespace seed
{
    static const uintptr_t VIEW_DEFAULT_NODE_MAX_SIZE = onut::max(
        sizeof(Node), 
        sizeof(Sprite), 
        sizeof(SpriteString), 
        sizeof(Emitter), 
        sizeof(SoundEmitter), 
        sizeof(MusicEmitter), 
        sizeof(Video),
        sizeof(TiledMapNode),
        sizeof(Effect));

    View::View()
        : m_nodePool(VIEW_DEFAULT_NODE_MAX_SIZE, VIEW_DEFAULT_NODE_COUNT)
        , m_currentButton(nullptr)
        , m_rootNode(nullptr)
        , m_size(640, 480)
    {
        memset(m_focusedButtons, 0, 4);
        memset(m_defaultFocusedButton, 0, 4);
    }


    View::~View()
    {

    }

    void View::Show()
    {
        // create the root node
        m_rootNode = new Node();
        SetSize(Vector2(OScreenWf, OScreenHf));
        OnShow();
    }

    void View::Hide()
    {
        // free all Nodes
        OnHide();
        DeleteNodes();
        m_buttons.clear();
        m_currentButton = nullptr;
        memset(m_focusedButtons, 0, 4);
        memset(m_defaultFocusedButton, 0, 4);
    }

    void View::Update()
    {
        // update nodes
        m_rootNode->Update();
        UpdatePhysics();
        OnUpdate();

        // update buttons interactions
        UpdateButtons();
        UpdateFocus();
    }

    void View::Render()
    {
        // render nodes
        m_rootNode->Render();
        OnRender();
    }

    void View::FocusButton(Button* in_button, int in_playerIndex)
    {
        if (in_button == m_focusedButtons[in_playerIndex - 1])
        {
            return;
        }

        if (!in_button->CanBeFocused(in_playerIndex))
        {
            return;
        }


        if (m_focusedButtons[in_playerIndex - 1])
        {
            OnButtonFocusLost(m_focusedButtons[in_playerIndex - 1], in_playerIndex);
        }

        m_focusedButtons[in_playerIndex - 1] = in_button;
        OnButtonFocused(in_button, in_playerIndex);
    }

    void View::UpdateFocus()
    {
        if (m_currentButton)
        {
            if (m_focusedButtons[0])
            {
                OnButtonFocusLost(m_focusedButtons[0], 1);
                if (m_focusedButtons[0]->IsPressed())
                {
                    OnButtonUp(m_focusedButtons[0]);
                    m_focusedButtons[0]->SetPressed(false);
                }
                m_focusedButtons[0] = nullptr;
            }

            // no need to continue if the mouse is already clicking buttons
            return;
        }

        // check if a player is touching a gamepad, or keyboard for player 1
        for (int playerIndex = 1; playerIndex <= 4; ++playerIndex)
        {
            Vector2 dir = GetDirectionFromInputDevices(playerIndex);
            if (dir.x != 0 || dir.y != 0)
            {
                Button* newFocusedButton = GetNextFocusedButton(dir, playerIndex);
                if (newFocusedButton)
                {
                    Button* currentlyFocusedButton = m_focusedButtons[playerIndex - 1];
                    if (currentlyFocusedButton)
                    {
                        OnButtonFocusLost(currentlyFocusedButton, playerIndex);
                        if (currentlyFocusedButton->IsPressed())
                        {
                            OnButtonUp(currentlyFocusedButton);
                            currentlyFocusedButton->SetPressed(false);
                        }
                        m_focusedButtons[playerIndex - 1] = nullptr;
                    }
                    OnButtonFocused(newFocusedButton, playerIndex);
                    m_focusedButtons[playerIndex - 1] = newFocusedButton;
                }
            }
            
            Button* focusedButton = m_focusedButtons[playerIndex - 1];
            if (focusedButton)
            {
                // verify if we are pressing 'enter' or default A button on a gamepad
                if (playerIndex == 1)
                {
                    // check for "enter"
                    if (OJustPressed(OINPUT_RETURN))
                    {
                        focusedButton->SetPressed(true);
                        OnButtonDown(focusedButton);
                    }
                    else if (OJustReleased(OINPUT_RETURN) && focusedButton->IsPressed())
                    {
                        focusedButton->SetPressed(false);
                        OnButtonUp(focusedButton);
                        SendCommand(seed::eAppCommand::APP_SPECIFIC, focusedButton->GetCmd());
                    }
                }

                // test for gamepad input
                if (OGamePadJustPressed(onut::GamePad::A, playerIndex - 1))
                {
                    focusedButton->SetPressed(true);
                    OnButtonDown(focusedButton);
                }
                else if (OGamePadJustReleased(onut::GamePad::A, playerIndex - 1))
                {
                    focusedButton->SetPressed(false);
                    OnButtonUp(focusedButton);
                    SendCommand(seed::eAppCommand::APP_SPECIFIC, focusedButton->GetCmd());
                }
            }
        }
    }

    Button* View::GetFocusedButton(int in_playerIndex)
    {
        return m_focusedButtons[in_playerIndex - 1];
    }

    void View::SetDefaultFocusedButton(Button* in_button, int in_playerIndex)
    {
        m_defaultFocusedButton[in_playerIndex - 1] = in_button;
    }

    void View::UpdateButtons()
    {
        // check for mouse click events
        if (OJustPressed(OINPUT_MOUSEB1))
        {
            // check if clicked inside a button
            for (Button* but : m_buttons)
            {
                if (IsInside(OMousePos, but->GetSprite()))
                {
                    m_currentButton = but;
                    m_currentButton->SetPressed(true);
                    OnButtonDown(m_currentButton);
                    return;
                }
            }
        }

        if (OJustReleased(OINPUT_MOUSEB1))
        {
            if (m_currentButton)
            {
                if (IsInside(OMousePos, m_currentButton->GetSprite()))
                {
                    m_currentButton->SetPressed(false);
                    OnButtonUp(m_currentButton);
                    SendCommand(seed::eAppCommand::APP_SPECIFIC, m_currentButton->GetCmd());
                }
            }
            m_currentButton = nullptr;
        }

        if (m_currentButton)
        {
            if (m_currentButton->IsPressed() && !IsInside(OMousePos, m_currentButton->GetSprite()))
            {
                m_currentButton->SetPressed(false);
                OnButtonUp(m_currentButton);
            }
            else if (!m_currentButton->IsPressed() && IsInside(OMousePos, m_currentButton->GetSprite()))
            {
                m_currentButton->SetPressed(true);
                OnButtonDown(m_currentButton);
            }
        }
    }

    bool View::IsInside(const Vector2& in_pos, Sprite* in_sprite)
    {
        Vector2 pos = in_sprite->GetAbsolutePosition();
        Vector2 size = Vector2(in_sprite->GetWidth() * in_sprite->GetScale().x, in_sprite->GetHeight() * in_sprite->GetScale().y);
        Vector2 align = in_sprite->GetAlign();
        pos.x += size.x * -align.x;
        pos.y += size.y * -align.y;

        if (in_pos.x > pos.x
            && in_pos.x < pos.x + size.x
            && in_pos.y > pos.y
            && in_pos.y < pos.y + size.y)
        {
            return true;
        }

        return false;
    }

    void View::VisitNodes(const VisitCallback& callback)
    {
        if (m_rootNode->VisitBackgroundChildren(callback)) return;
        if (callback(m_rootNode)) return;
        if (m_rootNode->VisitForegroundChildren(callback)) return;
    }

    void View::VisitNodesBackward(const VisitCallback& callback)
    {
        if (m_rootNode->VisitForegroundChildrenBackward(callback)) return;
        if (callback(m_rootNode)) return;
        if (m_rootNode->VisitBackgroundChildrenBackward(callback)) return;
    }

    void View::AddNode(Node* in_node, Node* in_parent, int in_zIndex)
    {
        Node* parentNode = in_parent ? in_parent : m_rootNode;
        parentNode->Attach(in_node, in_zIndex);
    }

    Node* View::CreateNode()
    {
        Node* newNode = m_nodePool.alloc<Node>();
        m_pooledNodes.push_back(newNode);
        return newNode;
    }

    Sprite* View::CreateSprite(const string& in_textureName)
    {
        OTexture* texture = OGetTexture(in_textureName.c_str());
        if (!texture)
        {
            OLogE("Invalid texture name specified to View::CreateSprite : " + in_textureName);
            return nullptr;
        }

        Sprite* newSprite = m_nodePool.alloc<Sprite>();
        newSprite->SetTexture(texture);

        m_pooledNodes.push_back(newSprite);
        return newSprite;
    }

    Sprite* View::CreateSpriteWithSpriteAnim(const string& in_animSource, const string& in_defaultAnim)
    {
        Sprite* newSprite = m_nodePool.alloc<Sprite>();
        newSprite->SetSpriteAnimSource(in_animSource);
        newSprite->SetSpriteAnim(in_defaultAnim);

        m_pooledNodes.push_back(newSprite);
        return newSprite;
    }

    SpriteString* View::CreateSpriteString(const string& in_fontName)
    {
        OFont* font = OGetBMFont(in_fontName.c_str());
        if (!font)  
        {
            OLogE("Invalid font name specified to View::AddSpriteString : " + in_fontName);
            //return nullptr; // We want to be able to put bad names without crashing all the things in the editor. Text should just not appear
        }

        SpriteString* newSpriteString = m_nodePool.alloc<SpriteString>();
        newSpriteString->SetFont(font);
        m_pooledNodes.push_back(newSpriteString);

        return newSpriteString;
    }

    Emitter* View::CreateEmitter(const string& in_fxName)
    {
        Emitter* newEmitter = m_nodePool.alloc<Emitter>();
        newEmitter->Init(in_fxName);
        m_pooledNodes.push_back(newEmitter);

        return newEmitter;
    }

    SoundEmitter* View::CreateSoundEmitter(const string& in_file)
    {
        SoundEmitter* newSoundEmitter = m_nodePool.alloc<SoundEmitter>();
        newSoundEmitter->Init(in_file);

        m_pooledNodes.push_back(newSoundEmitter);
        return newSoundEmitter;
    }

    SoundEmitter* View::CreateRandomSoundEmitter(const vector<string>& in_files)
    {
        SoundEmitter* newSoundEmitter = m_nodePool.alloc<SoundEmitter>();
        newSoundEmitter->Init(in_files);

        m_pooledNodes.push_back(newSoundEmitter);
        return newSoundEmitter;
    }

    MusicEmitter* View::CreateMusicEmitter()
    {
        MusicEmitter* newMusicEmitter = m_nodePool.alloc<MusicEmitter>();
        m_pooledNodes.push_back(newMusicEmitter);
        return newMusicEmitter;
    }

    Video* View::CreateVideo()
    {
        Video* newVideo = m_nodePool.alloc<Video>();
        m_pooledNodes.push_back(newVideo);
        return newVideo;
    }

    Effect* View::CreateEffect()
    {
        Effect* newEffect = m_nodePool.alloc<Effect>();
        m_pooledNodes.push_back(newEffect);
        return newEffect;
    }

    TiledMapNode* View::CreateTiledMapNode(const string& in_file)
    {
        TiledMapNode* newTiledMap = m_nodePool.alloc<TiledMapNode>();
        newTiledMap->Init(in_file);

        m_pooledNodes.push_back(newTiledMap);
        return newTiledMap;
    }

    Button* View::AddButton(Sprite* in_sprite, const string& in_cmd)
    {
        Button* newButton = new Button();
        newButton->SetSprite(in_sprite);
        newButton->SetCmd(in_cmd);
        m_buttons.push_back(newButton);
        return newButton;
    }

    Node* View::DuplicateNode(Node* in_node)
    {
        return in_node->Duplicate(m_nodePool, m_pooledNodes);
    }

    Node* View::FindNode(const string& in_name)
    {
        return GetRootNode()->FindNode(in_name);
    }

    void View::DeleteNode(Node* in_node)
    {
        Node* parent = in_node->GetParent();
        if (parent)
        {
            parent->Detach(in_node);
        }

        DeleteChildNodes(in_node->GetBgChildren());
        DeleteChildNodes(in_node->GetFgChildren());

        if (IsPooled(in_node))
        {
            m_nodePool.dealloc(in_node);
        }
    }

    void View::DeleteChildNodes(NodeVect& in_childVect)
    {
        // make a safe copy before deleting the children
        NodeVect copy = in_childVect;
        for (Node* n : copy)
        {
            DeleteNode(n);
        }
        in_childVect.clear();
    }

    void View::DeleteNodes()
    {
        DeleteNode(m_rootNode);
        delete m_rootNode;
        m_pooledNodes.clear();
        m_nodePool.clear();
    }

    bool View::IsPooled(Node* in_node)
    {
        for (Node* node : m_pooledNodes)
        {
            if (node == in_node)
            {
                return true;
            }
        }
        return false;
    }

    void View::SendCommand(eAppCommand in_command, const string& in_params)
    {
        m_queuedCommands.push_back(SCommand());
        SCommand& cmd = m_queuedCommands.back();

        cmd.m_command = in_command;
        cmd.m_params = onut::splitString(in_params, ',');
    }

    void View::SetSize(const Vector2& in_size)
    {
        m_size = in_size;
    }

    Vector2 View::GetDirectionFromInputDevices(int in_playerIndex)
    {
        if (in_playerIndex == 1)
        {
            // check keyboard for player 1
            if (OJustPressed(OINPUT_UP))
            {
                return Vector2(0, -1);
            }
            else if (OJustPressed(OINPUT_DOWN))
            {
                return Vector2(0, 1);
            }
            else if (OJustPressed(OINPUT_LEFT))
            {
                return Vector2(-1, 0);
            }
            else if (OJustPressed(OINPUT_RIGHT))
            {
                return Vector2(1, 0);
            }
        }

        // verify gamepads inputs
        if (OGamePadJustPressed(onut::GamePad::DPAD_UP, in_playerIndex - 1))
        {
            return Vector2(0, -1);
        }
        else if (OGamePadJustPressed(onut::GamePad::DPAD_DOWN, in_playerIndex - 1))
        {
            return Vector2(0, 1);
        }
        else if (OGamePadJustPressed(onut::GamePad::DPAD_LEFT, in_playerIndex - 1))
        {
            return Vector2(-1, 0);
        }
        else if (OGamePadJustPressed(onut::GamePad::DPAD_RIGHT, in_playerIndex - 1))
        {
            return Vector2(1, 0);
        }
        return Vector2();
    }

    Button* View::GetNextFocusedButton(const Vector2& in_dir, int in_playerIndex)
    {
        Button* focusedButton = m_focusedButtons[in_playerIndex - 1];
        if (!focusedButton)
        {
            // no button has focus, return the default
            return m_defaultFocusedButton[in_playerIndex - 1];
        }

        Vector2 focusedAbsPos = focusedButton->GetSprite()->GetAbsolutePosition();

        const float maxDot = -.7f;
        float bestDot = .5f;
        Button* bestButton = nullptr;
        ButtonVect candidates = GetPotentialCandidates(in_dir, in_playerIndex);
        for (Button* btn : candidates)
        {
            Vector2 spriteAbsPos = btn->GetSprite()->GetAbsolutePosition();
            // define the direction vector from this potential candidate to the currently focused sprite
            Vector2 sDir = focusedAbsPos - spriteAbsPos;
            sDir.Normalize();
            float dot = in_dir.Dot(sDir);
            //float dotDiff = fabsf(dot - bestDot);

            if (dot < maxDot)
            {
                bool canAccept = true;
                if (bestButton)
                {
                    // only accept this entry if it's closer to the currently focused button

                    if (Vector2::DistanceSquared(focusedAbsPos, spriteAbsPos) > Vector2::DistanceSquared(focusedAbsPos, bestButton->GetSprite()->GetAbsolutePosition()))
                    {
                        // can't accept it, it's too far
                        canAccept = false;
                    }
                }

                if (canAccept)
                {
                    bestDot = dot;
                    bestButton = btn;
                }
            }
        }
        return bestButton;
    }

    ButtonVect View::GetPotentialCandidates(const Vector2& in_dir, int in_playerIndex)
    {
        ButtonVect result;
        Button* focusedButton = m_focusedButtons[in_playerIndex - 1];
        Vector2 focusedAbsPos = focusedButton->GetSprite()->GetAbsolutePosition();
        for (Button* btn : m_buttons)
        {
            if (btn == focusedButton || !focusedButton->GetSprite()->GetVisible())
            {
                continue;
            }

            if (!btn->CanBeFocused(in_playerIndex))
            {
                continue;
            }

            Vector2 absPos = btn->GetSprite()->GetAbsolutePosition();
            if (in_dir.x < 0)
            {
                if (absPos.x < focusedAbsPos.x)
                {
                    if (!AlreadyInVector(btn, result))
                    {
                        result.push_back(btn);
                    }
                }
            }
            else if (in_dir.x >= 0)
            {
                if (absPos.x > focusedAbsPos.x)
                {
                    if (!AlreadyInVector(btn, result))
                    {
                        result.push_back(btn);
                    }
                }
            }


            if (in_dir.y < 0)
            {
                if (absPos.y < focusedAbsPos.y)
                {
                    if (!AlreadyInVector(btn, result))
                    {
                        result.push_back(btn);
                    }
                }
            }
            else if (in_dir.y >= 0)
            {
                if (absPos.y > focusedAbsPos.y)
                {
                    if (!AlreadyInVector(btn, result))
                    {
                        result.push_back(btn);
                    }
                }
            }
        }
        return result;
    }

    void View::Save(const string& filename)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement* pXMLView = doc.NewElement("View");
        pXMLView->SetAttribute("width", (int)m_size.x);
        pXMLView->SetAttribute("height", (int)m_size.y);
        doc.InsertFirstChild(pXMLView);

        const NodeVect& rootBGChildren = GetRootNode()->GetBgChildren();
        const NodeVect& rootFGChildren = GetRootNode()->GetFgChildren();

        for (auto pChild : rootBGChildren)
        {
            pXMLView->InsertEndChild(pChild->Serialize(&doc));
        }

        for (auto pChild : rootFGChildren)
        {
            pXMLView->InsertEndChild(pChild->Serialize(&doc));
        }

        doc.SaveFile(filename.c_str());
    }

    void View::Load(const string& filename)
    {
        tinyxml2::XMLDocument doc;
        doc.LoadFile(filename.c_str());

        tinyxml2::XMLElement* pXMLView = doc.FirstChildElement("View");
        assert(pXMLView);

        pXMLView->QueryFloatAttribute("width", &m_size.x);
        pXMLView->QueryFloatAttribute("height", &m_size.y);

        GetRootNode()->Deserialize(this, pXMLView);
    }

    bool View::AlreadyInVector(Button* in_button, ButtonVect& in_vector)
    {
        for (Button* btn : in_vector)
        {
            if (btn == in_button)
            {
                return true;
            }
        }
        return false;
    }

    PhysicsMgr& View::GetPhysics()
    {
        return m_physics;
    }

    PhysicsBody* View::CreateBoxPhysicsForNode(Node* in_node, bool in_static)
    {
        return m_physics.CreateBoxPhysicsForNode(in_node, in_static);
    }

    PhysicsBody* View::CreateCirclePhysicsForNode(Node* in_node, float in_radius, bool in_static)
    {
        return m_physics.CreateCirclePhysicsForNode(in_node, in_radius, in_static);
    }

    PhysicsBody* View::GetPhysicsForNode(Node* in_node)
    {
        return m_physics.GetBodyForNode(in_node);
    }

    void View::UpdatePhysics()
    {
        m_physics.Update();
    }

}
