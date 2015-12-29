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

    void View::UpdateButtons()
    {
        // check for mouse click events
        if (OJustPressed(OINPUT_MOUSEB1))
        {
            m_lastMouseDown = OMousePos;

            // check if clicked inside a button
            for (Button* but : m_buttons)
            {
                if (IsInside(m_lastMouseDown, but->GetSprite()))
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
                    m_currentButton->SetPressed(false);
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

    void View::visitNodes(const VisitCallback& callback)
    {
        for (Node* s : m_nodes)
        {
            if (s->visitBackgroundChildren(callback)) return;
            if (callback(s)) return;
            if (s->visitForegroundChildren(callback)) return;
        }
    }

    void View::visitNodesBackward(const VisitCallback& callback)
    {
        NodeVect::const_reverse_iterator end = m_nodes.rend();
        for (NodeVect::const_reverse_iterator it = m_nodes.rbegin(); it != end; ++it)
        {
            Node* s = *it;
            if (s->visitForegroundChildrenBackward(callback)) return;
            if (callback(s)) return;
            if (s->visitBackgroundChildrenBackward(callback)) return;
        }
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

    void View::AddButton(Sprite* in_sprite, const string& in_cmd)
    {
        Button* newButton = new Button();
        newButton->SetSprite(in_sprite);
        newButton->SetCmd(in_cmd);
        m_buttons.push_back(newButton);
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
}





