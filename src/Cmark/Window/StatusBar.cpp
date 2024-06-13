#include "StatusBar.h"

#include <QStatusBar>
#include <QString>

namespace CM
{
    QStatusBar * StatusBar::m_StatusBar = nullptr;

    void StatusBar::showMessage(const QString &message)
    {
        StatusBar::check();
        StatusBar::m_StatusBar->showMessage(message,0);
    }

    void StatusBar::showMessage(const QString &message, const int timeout)
    {
        StatusBar::check();
        m_StatusBar->showMessage(message,timeout);
    }

    void StatusBar::Init(QStatusBar *bar)
    {
        m_StatusBar = bar;
        StatusBar::check();
    }

    void StatusBar::clearMessage()
    {
        StatusBar::check();
        StatusBar::m_StatusBar->clearMessage();
    }

    void StatusBar::repaint()
    {
        StatusBar::check();
        StatusBar::m_StatusBar->repaint();
    }

    QStatusBar *StatusBar::statusBar()
    {
        StatusBar::check();
        return m_StatusBar;
    }

    void StatusBar::check()
    {
        if(m_StatusBar == nullptr)
        {
            throw std::runtime_error("Please Init StatusBar!");
        }
    }
} // CM
