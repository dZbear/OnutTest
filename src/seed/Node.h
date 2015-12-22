#pragma once

#include "onut.h"
#include "SeedGlobals.h"

namespace seed
{
	class Node
	{
	public:

        Node();
        virtual ~Node();

        virtual void    Update() {};
        virtual void    Render(Matrix* in_parentMatrix = nullptr) {};

        void		Attach(Node* in_newChild, int in_zIndex = INT_MAX);
        void		Detach(Node* in_child);
        Node*		GetParent();
        void        SetParent(Node* in_parent);
        void        SetZindex(int in_zIndex);
        int         GetZindex();

    protected:

        int             m_zIndex;
        Node*           m_parent;
        NodeVect		m_bgChildren;
        NodeVect		m_fgChildren;

    private:

        void		InsertNode(NodeVect& in_vect, Node* in_node, int in_zIndex);
        void		DetachChild(NodeVect& in_vect, Node* in_node);
	};
}