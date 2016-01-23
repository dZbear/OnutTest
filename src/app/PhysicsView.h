#pragma once
#include "View.h"

class PhysicsView : public seed::View
{
public:

    PhysicsView();
    virtual ~PhysicsView();

	virtual void OnShow();
	virtual void OnHide();
    virtual void OnUpdate();
    virtual void OnCollisionStart(seed::Node* in_nodeA, seed::Node* in_nodeB);
    virtual void OnCollisionEnd(seed::Node* in_nodeA, seed::Node* in_nodeB);
	
private:

    seed::Sprite*       m_dude;
    seed::Sprite*       m_box;

};