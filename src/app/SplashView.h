#pragma once
#include "View.h"

class SplashView final : public seed::View
{
public:
    void OnShow() override;

private:
    OTimer m_timer;
};
