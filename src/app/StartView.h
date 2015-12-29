#pragma once
#include "View.h"

class Button;
class StartView : public seed::View
{
public:

	StartView();
	virtual ~StartView();

	virtual void OnShow();
	virtual void OnHide();

    virtual void OnButtonDown(seed::Button* in_button);
    virtual void OnButtonUp(seed::Button* in_button);

    virtual void OnCommand(const string& in_cmd);
	
private:


};