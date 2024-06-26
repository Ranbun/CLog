#include <CMark.h>

#include "FileTreeDockWidget.h"
#include <Log/CLog.h>

#include <QTextEdit>
#include <QVBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>

#if _DEBUG
    #include <QDebug>
#endif

namespace CM
{
    FileTreeDockWidget::FileTreeDockWidget(const QString& title, QWidget* parent)
        : QDockWidget(title,parent)
        , m_FileSystemModel(nullptr)
    {

        setContentsMargins(0, 0, 0, 0);
        setFeatures(NoDockWidgetFeatures);

        /// show file tree
        m_TreeView = new QTreeView(this);
        m_TreeView->setWindowTitle("Directory Tree");

        setWidget(m_TreeView);

        setStyleSheet("QDockWidget { background-color: lightgray; }");

        connect(m_TreeView, &QTreeView::clicked, this, [this](const QModelIndex& index)
        {
            if(auto isDir = m_FileSystemModel->isDir(index))
            {
                return ;
            }
            const QString filePath = m_FileSystemModel->filePath(index);
            emit previewImage(filePath);
            CLog::Info(QString("Clicked file path: ") + filePath);
        });

        const auto lTitleBar = this->titleBarWidget();
        const auto lEmptyWidget = new QWidget();
        setTitleBarWidget(lEmptyWidget);
        delete lTitleBar;

        layout()->setContentsMargins(0,0,0,0);
        layout()->setSpacing(0);


        m_FileSystemModel = new QFileSystemModel;
        m_TreeView->setModel(m_FileSystemModel);

        m_FileSystemModel->setRootPath(QDir::currentPath());

        CLogInstance.Info(QDir::currentPath());

        QStringList filter;
        filter << "*.jpg";
        m_FileSystemModel->setNameFilters(filter);
        m_FileSystemModel->setNameFilterDisables(false);

        // m_TreeView->setRootIndex(m_FileSystemModel->index("./"));
        m_TreeView->setRootIndex(m_FileSystemModel->index("E:\\Github\\Cmark\\src\\Cmark\\sources\\pictures"));

    }

    FileTreeDockWidget::~FileTreeDockWidget() = default;

    void FileTreeDockWidget::New()
    {
        m_TreeView->setModel(nullptr);
        m_FileSystemModel->deleteLater();
        delete m_FileSystemModel;
        m_FileSystemModel = nullptr;
    }

    void FileTreeDockWidget::Open(const QDir &dir)
    {
        auto path = dir.absolutePath();
        if(!m_FileSystemModel)
        {
            m_FileSystemModel = new QFileSystemModel;
            m_FileSystemModel->setRootPath(path);
            m_TreeView->setModel(m_FileSystemModel);

            QStringList filter;
            filter <<"*.jpg";
            m_FileSystemModel->setNameFilters(filter);
            m_FileSystemModel->setNameFilterDisables(false);
        }
        m_FileSystemModel->setRootPath(path);
        m_TreeView->setRootIndex(m_FileSystemModel->index(path));
    }

    QString FileTreeDockWidget::rootImagePath() const
    {
        const auto rootDirIndex = m_TreeView->rootIndex();
        return m_FileSystemModel->rootPath();

    }
}
