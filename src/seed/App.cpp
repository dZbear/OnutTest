#include "App.h"
#include "Node.h"
#include "View.h"
#include "onut.h"

namespace seed
{
    vector<seed::SCommand> App::s_commands;

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
        float screenRatio = OScreenWf / OScreenHf;
        for (View* v : m_viewStack)
        {
            // Update the view root node transform to fit into the screen
            const Vector2& viewSize = v->GetSize();
            if (screenRatio >= 1.f)
            {
                float vRatio = OScreenHf / viewSize.y;
                v->GetRootNode()->SetScale(Vector2(vRatio));
                v->GetRootNode()->SetPosition(Vector2((OScreenWf - viewSize.x * vRatio) * .5f, 0.f));
            }
            else
            {
                float hRatio = OScreenWf / viewSize.x;
                v->GetRootNode()->SetScale(Vector2(hRatio));
                v->GetRootNode()->SetPosition(Vector2(0.f, (OScreenHf - viewSize.y * hRatio) * .5f));
            }

            v->Update();

            CommandVect& viewCommands = v->GetQueuedCommands();
            s_commands.insert(std::end(s_commands), std::begin(viewCommands), std::end(viewCommands));
            viewCommands.clear();
        }
        OnUpdate();
        ProcessCommands();
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

    void App::SendCommand(eAppCommand in_command, const string& in_params)
    {
        s_commands.push_back(SCommand());
        SCommand& cmd = s_commands.back();

        cmd.m_command = in_command;
        cmd.m_params = onut::splitString(in_params, ',');
    }

    void App::ProcessCommands()
    {
        for (SCommand& cmd : s_commands)
        {
            switch (cmd.m_command)
            {
                case eAppCommand::POP_VIEW:
                {
                    PopView();
                    break;
                }
                case eAppCommand::PUSH_VIEW:
                {
                    PushView(cmd.m_params[0]);
                    break;
                }
                case eAppCommand::REMOVE_VIEW:
                {
                    RemoveView(cmd.m_params[0]);
                    break;
                }
                case eAppCommand::SWITCH_VIEW:
                {
                    SwitchView(cmd.m_params[0]);
                    break;
                }
                case eAppCommand::APP_SPECIFIC:
                {
                    if (OnCommand(cmd.m_params[0]))
                    {
                        break;
                    }
                    else
                    {
                        // command not used by app, let's send it to the top view
                        if (m_viewStack.size() > 0)
                        {
                            View* v = m_viewStack.back();
                            v->OnCommand(cmd.m_params[0]);
                            break;
                        }
                    }
                }
            }
        }
        s_commands.clear();
    }
}



