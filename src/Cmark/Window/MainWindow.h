#ifndef CAMERAMARK_MAINWINDOW_H
#define CAMERAMARK_MAINWINDOW_H

#include <QMainWindow>
#include "CThread/ThreadPool.h"

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

    private:
        void InitWindowLayout();
        void InitUi();
        void InitMenu();
        void InitTool();
        void InitConnect();

    protected:

        /// widget
        std::shared_ptr<DisplayWidget> m_DisplayWidget{nullptr};
        std::shared_ptr<FileTreeDockWidget> m_LeftDockWidget{nullptr};
        std::shared_ptr<ImagePropertyDockWidget> m_RightDockWidget{nullptr};

        /// File Actions
        QAction* m_NewAction{nullptr};
        QAction* m_OpenDirectoryAction{nullptr};   ///< 打开文件夹
        QAction* m_BatchProcessImage{nullptr};

        /// Edit
        QAction * m_EditPreviewSceneLayoutAction{nullptr};

        ThreadPool * pool;

    signals:
        /**
         * @brief
         * @return
         */
        QString sigBatchProcessImagesRootPath();
        void sigWarning(QString info);

    };

} // CM

#endif //CAMERAMARK_MAINWINDOW_H
