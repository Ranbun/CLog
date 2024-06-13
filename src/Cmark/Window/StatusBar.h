#ifndef CAMERAMARK_STATUSBAR_H
#define CAMERAMARK_STATUSBAR_H

class QString;
class QStatusBar;

namespace CM
{
    class StatusBar
    {
    public:
        static void showMessage(const QString &message);
        static void showMessage(const QString &message,int timeout);
        static void Init(QStatusBar * bar);
        static void clearMessage();
        static void repaint();

        static QStatusBar * statusBar();

    private:
        static QStatusBar * m_StatusBar;
        static void check();
    };

} // CM

#endif //CAMERAMARK_STATUSBAR_H
