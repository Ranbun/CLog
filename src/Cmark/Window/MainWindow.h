#ifndef CAMERAMARK_MAINWINDOW_H
#define CAMERAMARK_MAINWINDOW_H

#include <QMainWindow>
#include <CThread/ThreadPool.h>

namespace CM
{
    class DisplayWidget;
    class FileTreeDockWidget;
    class ImagePropertyDockWidget;
}

namespace CM
{
    class MainWindow final :public  QMainWindow
    {
        Q_OBJECT
    public:
        MainWindow();

    protected:

        /// widget
        std::shared_ptr<DisplayWidget> m_DisplayWidget{ nullptr };
        std::shared_ptr<FileTreeDockWidget> m_FileTreeDockWidget{ nullptr };
        std::shared_ptr<ImagePropertyDockWidget> m_ImagePropertyDockWidget{ nullptr };

        /// File Actions
        QAction* m_NewAction{ nullptr };
        QAction* m_OpenDirectoryAction{ nullptr };   ///< 打开文件夹
        QAction* m_BatchProcessImage{ nullptr };
        QAction* m_OpenFile{ nullptr };

        /// Edit Actions
        QAction* m_EditPreviewSceneLayoutAction{ nullptr };

    protected:
        void resizeEvent(QResizeEvent* event) override;


    private:
        void InitWindowLayout();
        void InitUi();
        void InitMenu();
        void InitTool();
        void InitConnect();



    signals:
        QString sigBatchProcessImagesRootPath();
        void sigWarning(QString info);

    };

} // CM

#endif //CAMERAMARK_MAINWINDOW_H
