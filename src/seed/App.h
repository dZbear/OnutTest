#pragma once
#include "SeedGlobals.h"
#include "onut.h"

namespace seed
{
    class View;
    class App
    {
    public:

        App();
        virtual ~App();



        static void SendCommand(eAppCommand in_command, const string& in_params="");
        static vector<SCommand> s_commands;

        // to be overriden by your "Game Specific" App
        virtual void OnStart() {};
        virtual void OnUpdate() {};
        virtual void OnRender() {};
        virtual bool OnCommand(const string& in_cmd) { return false;  }

        // only called by seed sdk
        void    Start();
        void    Update();
        void    Render();


    protected:

        void    AddView(const string& in_viewName, View* in_newView);
        void    PushView(const string& in_viewName);
        void    PopView();
        void    RemoveView(const string& in_viewName);
        void    SwitchView(const string& in_viewName);

        View*   GetViewByName(const string& in_viewName);
        bool    IsViewOnStack(const string& in_viewName);

    private:

        // all views that can be shown
        ViewMap     m_views;

        // all the views currently shown, rendered first to last
        ViewStack   m_viewStack;

        void        ProcessCommands();

    };
}