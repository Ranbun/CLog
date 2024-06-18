#ifndef CAMERAMARK_DISPLAYWIDGET_H
#define CAMERAMARK_DISPLAYWIDGET_H

#include <QWidget>
#include <filesystem>


class QGraphicsView;
class QGraphicsScene;
class QGraphicsPixmapItem;

class SceneLayoutEditor;

namespace CM
{
    class PreViewImageScene;
}

namespace CM
{
    enum SceneIndex
    {
        None,
        PreviewScene,
        GenerateLogoScene
    };
}

namespace CM
{
    class DisplayWidget final : public QWidget
    {
        Q_OBJECT

    public:
        explicit DisplayWidget(QWidget* parent = nullptr);

    public:
        /**
         * @brief 打开空的工程
         * @param path 路径
         * @note connect with sigOpen
         */
        void open(const std::string& path) const;

        /**
         * @brief 预览文件路径下的图片
         * @param path 文件路径
         */
        void preViewImage(const std::string& path);

        /**
         * @brief save scene as Image
         * @param sceneIndex 要存储的场景的索引
         */
        void saveScene(SceneIndex sceneIndex);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        QGraphicsScene* m_PreviewImageScene{nullptr}; ///< 预览使用的场景

        QGraphicsScene* m_AddLogoScene{nullptr};  ///< 存储源尺寸大小的带logo的图片的场景

        /**
         * @brief 当前显示的视图
         */
        QGraphicsView* m_View{nullptr};

    signals:
        void sigOpen(const std::string & path);
        void sigCreated();
        void sigPreViewImage(const std::string& filePath);
        void sigPreViewLayoutSettingsPanel();
        void sigShowImage(size_t fileIndexCode);

    private:
        void InitConnect();

    };
} // CM

#endif //CAMERAMARK_DISPLAYWIDGET_H
