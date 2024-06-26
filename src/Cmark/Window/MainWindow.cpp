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

    void MainWindow::resizeEvent(QResizeEvent* event)
    {
        QMainWindow::resizeEvent(event);
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
            m_FileTreeDockWidget = std::shared_ptr<FileTreeDockWidget>(new FileTreeDockWidget("Dock Widget", this),
                                                                   []([[maybe_unused]] FileTreeDockWidget* w)
                                                                   {
                                                                   });
            m_FileTreeDockWidget->setWindowTitle("");

             addDockWidget(Qt::LeftDockWidgetArea, m_FileTreeDockWidget.get());
        }

        /// right dock widget
        {
            m_ImagePropertyDockWidget = std::shared_ptr<ImagePropertyDockWidget>(
                new ImagePropertyDockWidget(this), []([[maybe_unused]] ImagePropertyDockWidget* w)
                {
                });
            m_ImagePropertyDockWidget->setVisible(true);
            addDockWidget(Qt::LeftDockWidgetArea, m_ImagePropertyDockWidget.get());
        }

        splitDockWidget(m_FileTreeDockWidget.get(), m_ImagePropertyDockWidget.get(), Qt::Vertical);

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
            m_FileTreeDockWidget->New();
            Tools::ResourcesTools::destroy();
        });

        connect(m_OpenDirectoryAction, &QAction::triggered, [this]()
        {
            const auto workPath = emit sigBatchProcessImagesRootPath();
            const auto directoryPath = QFileDialog::getExistingDirectory(this,"Select Directory",workPath.isEmpty() ? QString("./"): workPath);
            if(directoryPath.isEmpty())
            {
                CLog::Warning<QString>("Can not found Path!");
                return ;
            }
            const QDir dir(directoryPath);
            m_FileTreeDockWidget->Open(dir);
            emit m_DisplayWidget->sigOpen(directoryPath.toStdString());
        });

        connect(m_BatchProcessImage, &QAction::triggered, this, [this]()
        {
            const QString rootPath = emit sigBatchProcessImagesRootPath();
            if(rootPath.isEmpty())
            {
                CLog::Warning<QString>("Can not found Path!");
                return ;
            }

            QDir dir(rootPath);
            BatchImageProcessor batchImageProcessor({rootPath});
            batchImageProcessor.Run();
        });

        /// 打开文件
        connect(m_OpenFile,&QAction::triggered,[this]()
        {
            const QString file = QFileDialog::getOpenFileName(this);
            const QFileInfo fileIns(file);
            if(!fileIns.exists())
            {
                return ;
            }
            CLogInstance.PrintMes<QString>(file);
            emit m_DisplayWidget->sigPreViewImage(file.toStdString());
            emit m_ImagePropertyDockWidget->sigShowProperty(file);
        });

        /// 获取批处理的文件目录
        connect(this, &MainWindow::sigBatchProcessImagesRootPath, m_FileTreeDockWidget.get(), [this]()-> QString
        {
            return m_FileTreeDockWidget->rootImagePath();
        });

        /// 预览图片
        connect(m_FileTreeDockWidget.get(), &FileTreeDockWidget::previewImage, [this](const QString& path)
        {
            const std::filesystem::path imagePath(path.toStdString());
            StatusBar::showMessage("preview image: " + path);
            StatusBar::repaint();

            emit m_DisplayWidget->sigPreViewImage(path.toStdString());
            emit m_ImagePropertyDockWidget->sigShowProperty(path);

        });

        /// 警告信息
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

        m_OpenFile = new QAction("Preview File");
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
    }
} // CM
