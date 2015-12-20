#pragma once
#include "View.h"
#include "Sprite.h"

class StartView : public seed::View
{
public:

	StartView();
	virtual ~StartView();

	virtual void OnCreate();
	virtual void OnShow();
	virtual void OnHide();
	
private:

	seed::Sprite*		m_testSprite;
	

};