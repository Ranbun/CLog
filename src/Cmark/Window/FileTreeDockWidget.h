#ifndef  CAMERAMARK_FILETREEDOCKWIDGET_H_
#define  CAMERAMARK_FILETREEDOCKWIDGET_H_

#include <QDockWidget>
#include <QTextEdit>
#include <QDir>

class QTreeView;
class QFileSystemModel;

namespace CM
{
    class FileTreeDockWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit FileTreeDockWidget(const QString & title, QWidget * parent = nullptr);
        ~FileTreeDockWidget() override;

        FileTreeDockWidget(const FileTreeDockWidget& other) = delete;
        FileTreeDockWidget(const FileTreeDockWidget&& other) = delete;

        FileTreeDockWidget& operator=(const FileTreeDockWidget& other) = delete;
        FileTreeDockWidget& operator=(const FileTreeDockWidget&& other) = delete;

        void New();
        void Open(const QDir &dir);

        /**
         * @brief 获取当前FileSystem Model 指向的文件路径
         * @return QString
         */
        QString rootImagePath() const;

    private:
        QTreeView * m_TreeView;
        QFileSystemModel * m_FileSystemModel;

    signals:
        void previewImage(const QString & path);

    };

}


#endif // ! CAMERAMARK_FILETREEDOCKWIDGET_H_
