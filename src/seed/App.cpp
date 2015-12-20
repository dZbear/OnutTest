#include "App.h"
#include "View.h"
#include "onut.h"

namespace seed
{
	App::App()
	{

	}

	App::~App()
	{

	}

	void App::Start()
	{
		OnStart();
	}

	void App::Update()
	{
		for (View* v : m_viewStack)
		{
			v->Update();
		}
		OnUpdate();
	}

	void App::Render()
	{
		ORenderer->clear(Color::Black);

		OSpriteBatch->begin();

		for (View* v : m_viewStack)
		{
			v->Render();
		}
		OnRender();

		OSpriteBatch->end();
	}

	void App::AddView(const string& in_viewName, View* in_newView)
	{
		in_newView->OnCreate();
		m_views[in_viewName] = in_newView;
	}

	void App::PushView(const string& in_viewName)
	{
		// make sure the view is not already on the stack
		RemoveView(in_viewName);

		ViewMap::const_iterator view = m_views.find(in_viewName);
		if (view == m_views.end())
		{
			OLogE("Invalid view name specified to App::PushView : " + in_viewName);
			return;
		}

		view->second->Show();
		m_viewStack.push_back(view->second);
	}

	void App::PopView()
	{
		if (m_viewStack.size() > 0)
		{
			View* view = m_viewStack.back();
			view->Hide();
			m_viewStack.pop_back();
		}
	}

	void App::SwitchView(const string& in_viewName)
	{
		PopView();
		PushView(in_viewName);
	}

	void App::RemoveView(const string& in_viewName)
	{
		View* view = GetViewByName(in_viewName);
		if (view)
		{
			for (size_t i = 0, size = m_viewStack.size(); i < size; ++i)
			{
				if (m_viewStack[i] == view)
				{
					view->Hide();
					m_viewStack.erase(m_viewStack.begin() + i);
					return;
				}
			}
		}
	}

	View* App::GetViewByName(const string& in_viewName)
	{
		ViewMap::const_iterator view = m_views.find(in_viewName);
		if (view == m_views.end())
		{
			OLogE("Invalid view name specified to App::GetViewByName : " + in_viewName);
			return nullptr;
		}
		return view->second;
	}

	bool App::IsViewOnStack(const string& in_viewName)
	{
		View* view = GetViewByName(in_viewName);
		if (!view)
		{
			OLogE("Invalid view name specified to App::IsViewOnStack : " + in_viewName);
			return false;
		}
		
		bool isOnStack = false;
		for (View* v : m_viewStack)
		{
			if (v == view)
			{
				isOnStack = true;
				break;
			}
		}
		return isOnStack;
	}
}



