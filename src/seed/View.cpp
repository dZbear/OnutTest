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
}





