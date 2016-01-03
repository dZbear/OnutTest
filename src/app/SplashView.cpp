#include "SplashView.h"

void SplashView::OnShow()
{
    // Load from file
    Load("../../assets/views/SplashView.xml");

    // Start a timer then switch to main menu
    m_timer.start(2.f, [this]
    {
        SendCommand(seed::eAppCommand::SWITCH_VIEW, "StartView");
    });
}
