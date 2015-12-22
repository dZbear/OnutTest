#pragma once
#include "SeedGlobals.h"
#include "onut.h"

namespace seed
{
	class Sprite;
	class View
	{
	public:

		View();
		virtual ~View();

		Sprite*	    AddSprite(const string& in_textureName, int in_zIndex=INT_MAX);
        void	    AddNode(Node* in_node, int in_zIndex = INT_MAX);
		void	    DeleteNode(Node* in_node);

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
		NodeVect		m_nodes;

        // keep track of stuff in the pool
        NodeVect        m_pooledNodes;

		void			InsertNode(Node* in_node, int in_zIndex);
        void            DeleteNodes();
        bool            IsPooled(Node* in_node);

		// sprite pool
		onut::Pool<true>			m_nodePool;


	};
}