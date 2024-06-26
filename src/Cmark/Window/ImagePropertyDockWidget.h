#ifndef CAMERAMARK_IMAGEPROPERTYDOCKWIDGET_H
#define CAMERAMARK_IMAGEPROPERTYDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>

namespace CM
{
    class ImagePropertyDockWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit ImagePropertyDockWidget(QWidget *parent = nullptr);


        ImagePropertyDockWidget(const ImagePropertyDockWidget&) = delete;
        ImagePropertyDockWidget(ImagePropertyDockWidget&&) = delete;

        ImagePropertyDockWidget & operator=(const ImagePropertyDockWidget&) = delete;
        ImagePropertyDockWidget & operator=(ImagePropertyDockWidget&&) = delete;

        ~ImagePropertyDockWidget() override = default;


    signals:
        void sigShowProperty(const QString & path);

    protected:

        void resizeEvent(QResizeEvent* event) override;

    protected:
        void onSigShowProperty(const QString & path);

    private:
        void init();
        void initUi();
        void initLayout();
        void initConnect();

    private:
        QLabel*  m_FilePathLabel;        ///< file path 
        QLabel*  m_FilePathTitleLabel;   ///< file path 
        QLabel*  m_DimensionTitle;             ///< 维度
        QLabel*  m_Dimension;             ///< 维度

        QString m_FilePath;

    };

} // CM

#endif //CAMERAMARK_IMAGEPROPERTYDOCKWIDGET_H
