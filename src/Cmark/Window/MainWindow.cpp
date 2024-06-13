#include <CMark.h>

#include "MainWindow.h"

#include <future>

#include <File/BatchImageProcess.h>
#include <File/ResourcesTool.h>
#include <Log/CLog.h>

#include "DisplayWidget.h"
#include "FileTreeDockWidget.h"
#include "ImagePropertyDockWidget.h"
#include "StatusBar.h"

#include <QFileDialog>
#include <QMenuBar>
#include <QPainter>
#include <QToolBar>
#include <QObject>

namespace CM
{
    MainWindow::MainWindow()
        : QMainWindow(nullptr)
    {
        InitUi();
    }

    void MainWindow::InitWindowLayout()
    {
        /// Central Widget
        m_DisplayWidget = std::shared_ptr<DisplayWidget>(new DisplayWidget, []([[maybe_unused]] DisplayWidget* w)
        {
        });
        this->setCentralWidget(m_DisplayWidget.get());
        this->setMinimumSize({960, 720}); ///< resize Window
        m_DisplayWidget->setMinimumSize(640, 480);
        /// left dock widget
        {
            m_LeftDockWidget = std::shared_ptr<FileTreeDockWidget>(new FileTreeDockWidget("Dock Widget", this),
                                                                   []([[maybe_unused]] FileTreeDockWidget* w)
                                                                   {
                                                                   });
            m_LeftDockWidget->setWindowTitle("");

            addDockWidget(Qt::LeftDockWidgetArea, m_LeftDockWidget.get());
        }

        /// right dock widget
        {
            m_RightDockWidget = std::shared_ptr<ImagePropertyDockWidget>(
                new ImagePropertyDockWidget(this), []([[maybe_unused]] ImagePropertyDockWidget* w)
                {
                });
            m_RightDockWidget->setWindowTitle("");
            m_RightDockWidget->setVisible(false);

            // addDockWidget(Qt::RightDockWidgetArea, m_rightDockWidget.get());
        }

        setContentsMargins(0, 0, 0, 0);
    }

    void MainWindow::InitUi()
    {
        InitWindowLayout();
        InitMenu();
        InitTool();
        InitConnect();

        StatusBar::showMessage("Initialized!");
    }

    void MainWindow::InitConnect()
    {
        connect(m_NewAction, &QAction::triggered, [this]()
        {
            m_LeftDockWidget->New();
            Tools::ResourcesTools::destroy();
        });

        connect(m_OpenDirectoryAction, &QAction::triggered, [this]()
        {
            const auto directoryPath = QFileDialog::getExistingDirectory(this);
            const auto path = std::filesystem::path(directoryPath.toStdString());
            m_LeftDockWidget->showMessage(directoryPath.toStdString());
            emit m_DisplayWidget->sigOpen(directoryPath.toStdString());
        });

        connect(m_BatchProcessImage, &QAction::triggered, this, [this]()
        {
            const QString rootPath = emit sigBatchProcessImagesRootPath();
            BatchImageProcessor batchImageProcessor({rootPath});
            batchImageProcessor.Run();
        });

        connect(m_OpenFile,&QAction::triggered,[this]()
        {
            QString file = QFileDialog::getOpenFileName(this);
            QFileInfo fileIns(file);
            if(!fileIns.exists())
            {
                return ;
            }
            CLogInstance.PrintMes<QString>(file);
            emit m_DisplayWidget->sigPreViewImage(file.toStdString());
        });

        connect(this, &MainWindow::sigBatchProcessImagesRootPath, m_LeftDockWidget.get(), [this]()-> QString
        {
            return m_LeftDockWidget->rootImagePath();
        });

        connect(m_LeftDockWidget.get(), &FileTreeDockWidget::previewImage, [this](const QString& path)
        {
            const std::filesystem::path imagePath(path.toStdString());
            StatusBar::showMessage("preview image: " + path);
            StatusBar::repaint();
            emit m_DisplayWidget->sigPreViewImage(path.toStdString());
        });

        connect(this, &MainWindow::sigWarning, this, [](const QString& info)
        {
            CLog::Warning(info);
        }, Qt::QueuedConnection);
    }

    void MainWindow::InitMenu()
    {
        const auto status = statusBar(); ///< 使用内置方法创建 status bar
        CM::StatusBar::Init(status);

        const auto currentMenuBar = menuBar();

        const auto file = new QMenu("File(&F)");
        currentMenuBar->addMenu(file);

        m_NewAction = new QAction("New");
        m_NewAction->setIcon(QIcon("./sources/icons/new.png"));
        m_NewAction->setToolTip(tr("set FileSystem Empty "));
        m_NewAction->setShortcut({"Ctrl+N"});
        file->addAction(m_NewAction);

        m_OpenFile = new QAction("preview File");
        file->addAction(m_OpenFile);
        m_OpenFile->setIcon(QIcon("./sources/icons/openFile.png"));

        m_OpenDirectoryAction = new QAction("Open Directory");
        m_OpenDirectoryAction = new QAction("Open Directory");
        m_OpenDirectoryAction->setToolTip(tr("Open Directory"));
        m_OpenDirectoryAction->setShortcut({"Ctrl+P"});
        m_OpenDirectoryAction->setIcon(QIcon("./sources/icons/openDirectory.png"));
        file->addAction(m_OpenDirectoryAction);

        m_BatchProcessImage = new QAction("Process All");
        m_BatchProcessImage->setToolTip(tr("Process All Image Files"));
        m_BatchProcessImage->setIcon(QIcon("./sources/icons/multiProcess.png"));
        m_BatchProcessImage->setShortcut({"Ctrl+Shift+A"});
        file->addAction(m_BatchProcessImage);

        const auto Edit = new QMenu("Edit(&E)");
        // MenuBar->addMenu(Edit);
        m_EditPreviewSceneLayoutAction = new QAction("Layout Setting");
        m_EditPreviewSceneLayoutAction->setToolTip(tr("Preview Scene Layout Setting"));
        m_EditPreviewSceneLayoutAction->setShortcut({"Ctrl+E"});
        m_EditPreviewSceneLayoutAction->setIcon(QIcon("./sources/icons/previewSceneLayoutsettings.png"));
        Edit->addAction(m_EditPreviewSceneLayoutAction);
    }

    void MainWindow::InitTool()
    {
        const auto toolBar = addToolBar("toolbar");

        toolBar->setFloatable(false);
        toolBar->setMovable(false);
        toolBar->setIconSize({16, 16});

        toolBar->addAction(m_OpenFile);
        toolBar->addAction(m_OpenDirectoryAction);

        /// 添加分割线
        toolBar->addSeparator();

        /// save preview image
        {
            const auto savePreviewImageAction = toolBar->addAction("Save preview");
            savePreviewImageAction->setToolTip("Save preview Image");
            QPixmap previewSceneSaveIcon("./sources/icons/save.png");
            previewSceneSaveIcon = previewSceneSaveIcon.scaled({16, 16}, Qt::KeepAspectRatio);
            savePreviewImageAction->setIcon(previewSceneSaveIcon);

            connect(savePreviewImageAction, &QAction::triggered, [this]()
            {
                m_DisplayWidget->saveScene(SceneIndex::PreviewScene);
            });
        }

        /// save it
#if  0
        {
            const auto save = toolBar->addAction("Add Logo Save");
            save->setToolTip("Save as Image with logo");
            QPixmap previewSceneSaveIcon("./sources/icons/save.png");
            previewSceneSaveIcon = previewSceneSaveIcon.scaled({16, 16}, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            save->setIcon(previewSceneSaveIcon);
            save->setVisible(true);
            connect(save, &QAction::triggered, [this]()
            {
                m_DisplayWidget->saveScene(SceneIndex::GenerateLogoScene);
            });
        }
#endif
    }
} // CM
