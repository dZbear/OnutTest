#include "View.h"
#include "Sprite.h"
#include "SpriteString.h"
#include "Button.h"
#include "onut.h"

#define VIEW_DEFAULT_NODE_COUNT 64
#define VIEW_DEFAULT_NODE_MAX_SIZE 512

namespace seed
{
    View::View()
        : m_nodePool(VIEW_DEFAULT_NODE_MAX_SIZE, VIEW_DEFAULT_NODE_COUNT)
        , m_currentButton(nullptr)
    {
        memset(m_focusedButtons, 0, 4);
        memset(m_defaultFocusedButton, 0, 4);
    }


    View::~View()
    {

    }

    void View::Show()
    {
        OnShow();
    }

    void View::Hide()
    {
        // free all Nodes
        DeleteNodes();
        OnHide();
        m_currentButton = nullptr;
        memset(m_focusedButtons, 0, 4);
        memset(m_defaultFocusedButton, 0, 4);
    }

    void View::Update()
    {
        // update nodes
        for (Node* s : m_nodes)
        {
            s->Update();
        }
        OnUpdate();

        // update buttons interactions
        UpdateButtons();
        UpdateFocus();
    }

    void View::Render()
    {
        // render nodes
        for (Node* s : m_nodes)
        {
            if (!s->GetParent())
            {
                // render this sprite if it doesn't have a parent
                s->Render();
            }
        }
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

    Sprite* View::AddSprite(const string& in_textureName, int in_zIndex)
    {
        OTexture* texture = OGetTexture(in_textureName.c_str());
        if (!texture)
        {
            OLogE("Invalid texture name specified to View::AddSprite : " + in_textureName);
            return nullptr;
        }

        Sprite* newSprite = m_nodePool.alloc<Sprite>();
        newSprite->SetZindex(in_zIndex);
        newSprite->SetTexture(texture);
        if (in_zIndex == INT_MAX)
        {
            m_nodes.push_back(newSprite);
        }
        else
        {
            InsertNode(newSprite, in_zIndex);
        }
        m_pooledNodes.push_back(newSprite);
        return newSprite;
    }

    SpriteString* View::AddSpriteString(const string& in_fontName, Node* in_parent, int in_zIndex)
    {
        OFont* font = OGetBMFont(in_fontName.c_str());
        if (!font)
        {
            OLogE("Invalid font name specified to View::AddSpriteString : " + in_fontName);
            return nullptr;
        }

        SpriteString* newSpriteString = m_nodePool.alloc<SpriteString>();
        newSpriteString->SetZindex(in_zIndex);
        newSpriteString->SetFont(font);
        m_pooledNodes.push_back(newSpriteString);

        if (in_parent)
        {
            in_parent->Attach(newSpriteString, in_zIndex);
        }
        else
        {
            if (in_zIndex == INT_MAX)
            {
                m_nodes.push_back(newSpriteString);
            }
            else
            {
                InsertNode(newSpriteString, in_zIndex);
            }
        }
        return newSpriteString;
    }

    Button* View::AddButton(Sprite* in_sprite, const string& in_cmd)
    {
        Button* newButton = new Button();
        newButton->SetSprite(in_sprite);
        newButton->SetCmd(in_cmd);
        m_buttons.push_back(newButton);
        return newButton;
    }

    void View::DeleteNode(Node* in_node)
    {
        for (size_t i = 0, size = m_nodes.size(); i < size; ++i)
        {
            if (m_nodes[i] == in_node)
            {
                Node* parent = in_node->GetParent();
                if (parent)
                {
                    parent->Detach(in_node);
                }
                m_nodes.erase(m_nodes.begin() + i);
                if (IsPooled(in_node))
                {
                    m_nodePool.dealloc(in_node);
                }
                return;
            }
        }
    }

    void View::InsertNode(Node* in_node, int in_zIndex)
    {
        for (size_t i = 0, size = m_nodes.size(); i < size; ++i)
        {
            if (m_nodes[i]->GetZindex() > in_zIndex)
            {
                // let's insert before this one
                m_nodes.insert(m_nodes.begin() + i, in_node);
                return;
            }
        }

        // if we're here it means we didnt find any suitable place for it, just insert at the end
        m_nodes.push_back(in_node);
    }

    void View::DeleteNodes()
    {
        for (size_t i = 0, size = m_nodes.size(); i < size; ++i)
        {
            if (IsPooled(m_nodes[i]))
            {
                m_nodePool.dealloc(m_nodes[i]);
            }
            else
            {
                delete m_nodes[i];
            }
        }
        m_nodes.clear();
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
}





/*
Sprite* FocusMgr::GetNextFocusedSprite(const Vector2f& in_touchStart, const Vector2f& in_touchEnd)
{
    if (!m_focusedSprite)
    {
        // can't find the 'next' focused sprite without an actual focused sprite
        return 0;
    }

    Vector2f focusedAbsPos = m_focusedSprite->GetAbsolutePosition() + m_focusedSprite->GetFocusOffset();
    // define the direction unit vector the user is swipping to
    Vector2f dir = (in_touchEnd - in_touchStart).Normalize();

    const float maxDot = -.7f;
    float bestDot = .5f;
    Sprite* bestSprite = 0;
    vector<Sprite*> candidates = GetPotentialCandidates(dir);
    for (Sprite* s : candidates)
    {
        Vector2f spriteAbsPos = s->GetAbsolutePosition() + s->GetFocusOffset();
        // define the direction vector from this potential candidate to the currently focused sprite
        Vector2f sDir = (focusedAbsPos - spriteAbsPos).Normalize();
        float dot = dir.Dot(sDir);
        //float dotDiff = fabsf(dot - bestDot);

        if (dot < maxDot)
        {
            bool canAccept = true;
            if (bestSprite)
            {
                // only accept this entry if it's closer to the currently focused sprite
                if (focusedAbsPos.DistanceSqr(spriteAbsPos) > focusedAbsPos.DistanceSqr(bestSprite->GetAbsolutePosition() + bestSprite->GetFocusOffset()))
                {
                    // can't accept it, it's farther to the
                    canAccept = false;
                }
            }

            if (canAccept)
            {
                bestDot = dot;
                bestSprite = s;
            }
        }
    }
    return bestSprite;
}

vector<Sprite*> FocusMgr::GetPotentialCandidates(const Vector2f in_dir)
{
    vector<Sprite*> result;
    Vector2f focusedAbsPos = m_focusedSprite->GetAbsolutePosition() + m_focusedSprite->GetFocusOffset();
    for (Sprite* s : m_sprites)
    {
        if (!s)
        {
            continue;
        }

        if (s == m_focusedSprite || !s->GetVisible())
        {
            continue;
        }

        Vector2f absPos = s->GetAbsolutePosition() + s->GetFocusOffset();
        if (in_dir.x < 0)
        {
            if (absPos.x < focusedAbsPos.x)
            {
                if (!AlreadyInVector(s, result))
                {
                    result.push_back(s);
                }
            }
        }
        else if (in_dir.x >= 0)
        {
            if (absPos.x > focusedAbsPos.x)
            {
                if (!AlreadyInVector(s, result))
                {
                    result.push_back(s);
                }
            }
        }


        if (in_dir.y < 0)
        {
            if (absPos.y < focusedAbsPos.y)
            {
                if (!AlreadyInVector(s, result))
                {
                    result.push_back(s);
                }
            }
        }
        else if (in_dir.y >= 0)
        {
            if (absPos.y > focusedAbsPos.y)
            {
                if (!AlreadyInVector(s, result))
                {
                    result.push_back(s);
                }
            }
        }
    }
    return result;
}

bool FocusMgr::AlreadyInVector(Sprite* in_sprite, vector<Sprite*>& in_vector)
{
    for (Sprite* s : in_vector)
    {
        if (s == in_sprite)
        {
            return true;
        }
    }
    return false;
}
*/