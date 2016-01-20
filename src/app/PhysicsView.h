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
	
private:

    seed::Sprite*       m_dude;
    seed::Sprite*       m_box;

};