#include <QApplication>

#include <File/LogoManager.h>
#include <Log/CLog.h>
#include <Window/MainWindow.h>

int main(int argc,char ** argv)
{
    QApplication app(argc,argv);
    CM::MainWindow manApp;

    CLogInstance.Init();
    CM::LogoManager::Init();

    manApp.resize(720,560);
    manApp.setWindowTitle("Camera Mark");
    manApp.show();

    return QApplication::exec();
}

