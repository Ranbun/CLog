#include "ImagePropertyDockWidget.h"
#include <Log/CLog.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>

namespace CM
{
    namespace ToolFunc
    {

        QString ElidedFilePathStr(const QString& prefixStr, const QString& str, const QFont& font, const QSize& widgetSize)
        {
            const auto w = widgetSize.width();

            const QFontMetrics metrics(font);
            const auto calcWordsW = QFontMetrics(font);
            const auto wordsW = calcWordsW.horizontalAdvance(prefixStr);
            auto elided = metrics.elidedText(str, Qt::ElideRight, w - wordsW);
            return elided;
        }

    }

    ImagePropertyDockWidget::ImagePropertyDockWidget(QWidget *parent)
    : QDockWidget(parent)
    {
        setContentsMargins(0, 0, 0, 0);
        setFeatures(NoDockWidgetFeatures);
        setMinimumHeight(200);
        setFixedWidth(280);

        setWindowTitle("");
        setTitleBarWidget(new QWidget());

        init();
    }

    void ImagePropertyDockWidget::initConnect()
    {
        QObject::connect(this,&ImagePropertyDockWidget::sigShowProperty, this, [this](const QString & fileName)
        {
            onSigShowProperty(fileName);

        });
    }

    void ImagePropertyDockWidget::initLayout()
    {
        const auto vbox = new QVBoxLayout(nullptr);

        
        {/// file path
            const auto filePathHBox = new QHBoxLayout;
            filePathHBox->setSpacing(5);
            filePathHBox->setMargin(0);
            filePathHBox->addSpacing(5);
            filePathHBox->addWidget(m_FilePathTitleLabel);
            filePathHBox->addWidget(m_FilePathLabel);
            filePathHBox->addStretch(1);
            vbox->addItem(filePathHBox);
        }

        {/// dimension

            const auto dimensionHBox = new QHBoxLayout;
            dimensionHBox->setSpacing(5);   
            dimensionHBox->setMargin(0);
            dimensionHBox->addSpacing(5);
            dimensionHBox->addWidget(m_DimensionTitle);
            dimensionHBox->addWidget(m_Dimension);
            dimensionHBox->addStretch(1);
            vbox->addItem(dimensionHBox);
        }

        vbox->setSpacing(5);
        vbox->setMargin(0);
        vbox->addStretch(1);

        const auto w = new QWidget(this);
        w->setLayout(vbox);
        setWidget(w);
    }

    void ImagePropertyDockWidget::init()
    {
        initUi();
        initLayout();
        initConnect();
    }

    void ImagePropertyDockWidget::initUi()
    {
        /// TODO: 目前没有什么好的办法解决这个布局的bug setText 会增加窗口的宽度
        m_FilePathTitleLabel = new QLabel("File Path: "); m_FilePathTitleLabel->setFixedWidth(66);
        m_FilePathLabel = new QLabel(); m_FilePathLabel->setFixedWidth(180);
        m_DimensionTitle = new QLabel("Dimension: ");
        m_Dimension = new QLabel();
    }

    void ImagePropertyDockWidget::resizeEvent(QResizeEvent* event)
    {

        m_FilePathLabel->setMaximumWidth(width() - m_FilePathTitleLabel->maximumWidth());

        const auto currentSize = event->size();
        const auto res = ToolFunc::ElidedFilePathStr(m_FilePathTitleLabel->text(), m_FilePath,
            m_FilePathLabel->font(), currentSize);
        m_FilePathLabel->setText(res);

        QDockWidget::resizeEvent(event);
    }

    void ImagePropertyDockWidget::onSigShowProperty(const QString &path)
    {

        m_FilePath = path;

        const auto currentSize = this->size();
        const auto res = ToolFunc::ElidedFilePathStr(m_FilePathTitleLabel->text(),m_FilePath,
            m_FilePathLabel->font(), currentSize);

        m_FilePathLabel->setText(res);
        m_FilePathLabel->setToolTip(m_FilePath);

    }

} // CM
