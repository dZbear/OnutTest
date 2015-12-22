#include "Node.h"

namespace seed
{
    Node::Node()
		: m_parent(nullptr)
	{
	}

    Node::~Node()
	{

	}

    void Node::SetZindex(int in_zIndex)
	{
		m_zIndex = in_zIndex;
	}

    int Node::GetZindex()
	{
		return m_zIndex;
	}

    void Node::Attach(Node* in_newChild, int in_zIndex)
	{
		if (in_zIndex < 0)
		{
			InsertNode(m_bgChildren, in_newChild, in_zIndex);
		}
		else
		{
            InsertNode(m_fgChildren, in_newChild, in_zIndex);
		}
		in_newChild->SetParent(this);
		in_newChild->SetZindex(in_zIndex);
	}

    void Node::Detach(Node* in_child)
	{
		if (in_child->GetZindex() < 0)
		{
			DetachChild(m_bgChildren, in_child);
		}
		else
		{
			DetachChild(m_fgChildren, in_child);
		}
		in_child->SetParent(nullptr);
	}

    void Node::InsertNode(NodeVect& in_vect, Node* in_node, int in_zIndex)
	{
		if (in_zIndex < 0)
		{
			// inserted in background children
			for (size_t i = 0, size = in_vect.size(); i < size; ++i)
			{
				if (in_vect[i]->GetZindex() < in_zIndex)
				{
					// let's insert before this one
                    in_vect.insert(in_vect.begin() + i, in_node);
					return;
				}
			}
		}
		else
		{
			// inserted in foreground children
			for (size_t i = 0, size = in_vect.size(); i < size; ++i)
			{
				if (in_vect[i]->GetZindex() > in_zIndex)
				{
					// let's insert before this one
                    in_vect.insert(in_vect.begin() + i, in_node);
					return;
				}
			}
		}
		// if we're here it means we didnt find any suitable place for it, just insert at the end
        in_vect.push_back(in_node);
	}

    void Node::DetachChild(NodeVect& in_vect, Node* in_node)
	{
		// inserted in background children
		for (size_t i = 0, size = in_vect.size(); i < size; ++i)
		{
            if (in_vect[i] == in_node)
			{
				// let's insert before this one
				in_vect.erase(in_vect.begin() + i);
				return;
			}
		}
	}

    void Node::SetParent(Node* in_parent)
	{
		m_parent = in_parent;
	}

    Node* Node::GetParent()
	{
		return m_parent;
	}
}





