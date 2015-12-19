#pragma once
#include "App.h"

class ONutTestApp : public seed::App
{
public:

	ONutTestApp();
	virtual ~ONutTestApp();

	virtual void OnStart();
	virtual void OnUpdate();
	virtual void OnRender();

private:
	

};