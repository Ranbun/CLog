#include <CMark.h>

#include "DisplayWidget.h"

#include <QFileDialog>
#include <QGraphicsView>
#include <QImage>
#include <QMessageBox>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <Base/ImagePack.h>
#include <File/ImageProcess/ImageProcess.h>

#include <File/LogoManager.h>
#include <File/PictureManager.h>
#include <File/Resolver/EXIFResolver.h>
#include "Scene/LifeSizeImageScene.h"
#include <Scene/PreViewImageScene.h>


namespace
{
    [[maybe_unused]] auto ImageSaveDefaultName() -> QString
    {
        const QDateTime currentDateTime = QDateTime::currentDateTime();
        auto outputName = currentDateTime.toString("yyyy-MM-dd__HHHmmMssS");

        constexpr std::hash<std::string> nameGenerator;
        const auto nameCode = nameGenerator(outputName.toStdString());
        outputName = outputName + "__" + QString::number(nameCode);
        return { outputName };
    }
}

namespace CM
{
    DisplayWidget::DisplayWidget(QWidget* parent)
        : QWidget(parent)
          , m_PreviewImageScene(new PreViewImageScene)
          , m_AddLogoScene(new LifeSizeImageScene)
          , m_View(new QGraphicsView)
    {

        qRegisterMetaType<std::string>("std::string");

        m_View->setScene(m_PreviewImageScene);
        connect(this, &DisplayWidget::sigCreated, this, [ parent= this, view = m_View ]()
        {
            view->setParent(parent);
        });

        m_View->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        m_View->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        m_View->setAlignment(Qt::AlignCenter);

        m_View->resize(960, 720);
        m_PreviewImageScene->setSceneRect(0, 0, m_View->rect().width(), m_View->rect().height());

        emit sigCreated();

        InitConnect();
    }

    void DisplayWidget::open(const std::string& path) const
    {
        assert(this);
        /// TODO:: maybe we need do something!
    }

    void DisplayWidget::preViewImage(const std::string& path)
    {

        using PictureManagerInterFace = CM::PictureManager;
        const auto fileIndexCode = ImageProcess::generateFileIndexCode(path);

        /// 如果图片被加载出来
        if(PictureManagerInterFace::getImage(fileIndexCode) != nullptr)
        {
            emit sigShowImage(fileIndexCode);
        }

        {
            /// load file as QByteArray
            auto [w,h] = SceneLayoutSettings::fixPreViewImageSize();
            auto data = ImageProcess::loadFile(QString::fromStdString(path));
            const ImagePack loadImagePack{ fileIndexCode,data,path,std::make_shared<std::mutex>(),{w,h}};

            /// load image

            PictureManagerInterFace::loadImage(loadImagePack,true);
            EXIFResolver::resolver(loadImagePack, false);

            const auto loadF = std::async(std::launch::async, PictureManagerInterFace::loadImage,std::ref(loadImagePack),true);
            const auto resolverF = std::async(std::launch::async, EXIFResolver::resolver, std::ref(loadImagePack),true);

            ///  等待异步任务完成
            loadF.wait();
            resolverF.wait();

            data->clear();
            data.reset();
        }

        /// 加载logo
        const auto cameraIndex = LogoManager::resolverCameraIndex(EXIFResolver::ExifItem(fileIndexCode,ExifKey::CameraMake));
        LogoManager::loadCameraLogo(cameraIndex);

        emit sigShowImage(fileIndexCode);

#if  0
        /// 设置单张图片存储的显示资源
        {
            const auto logoScene = dynamic_cast<LifeSizeImageScene*>(m_AddLogoScene);
            logoScene->resetStatus();
            logoScene->resetPreviewImageTarget(*preViewImage, fileIndexCode);
            logoScene->resetTexItemsPlainText(exifInfos);
            logoScene->resetLogoPixmap(previewImageLogo, cameraIndex);
        }
#endif
    }

    void DisplayWidget::resizeEvent(QResizeEvent* event)
    {
        const auto windowSize = event->size();
        m_View->resize(windowSize); ///< resize view

        /// 设置视图显示的场景的大小
        /// 设置视图观察的场景的观察点
        {
            const auto bound = m_PreviewImageScene->sceneRect();
            m_View->setSceneRect(bound); // 设置场景矩形
            m_View->fitInView(bound, Qt::KeepAspectRatio);
        }

        QWidget::resizeEvent(event);
    }

    void DisplayWidget::saveScene(const SceneIndex sceneIndex)
    {
        auto saveAsFile = [](const std::shared_ptr<QImage>& image, const QString& filePath)
        {
            ImageProcess::save(image, filePath);
        };

        auto save = [this,saveAsFile](QGraphicsScene* scene)
        {
            if (!scene) return;

            scene->clearSelection(); // Selections would also render to the file

            const auto rect = scene->sceneRect();
            const auto iSize = rect.size().toSize();
            auto image = std::make_shared<QImage>(iSize, QImage::Format_ARGB32);

            image->fill(Qt::white); // Start all pixels white

            QPainter painter(image.get());
            scene->render(&painter);
            painter.end();

            QFileDialog getFileDialog(this, tr("Save File"),
                                "./" + ImageSaveDefaultName(),
                                      tr("Images (*.png);;Images (*.xpm);;Images (*.jpg);;All Files (*)"));
            getFileDialog.setOption(QFileDialog::DontUseCustomDirectoryIcons);

            auto fileName = ImageSaveDefaultName() + ".png";


            switch (getFileDialog.exec())
            {
                case  QFileDialog::Accepted:
                {
                    auto suffix = getFileDialog.selectedNameFilter();
                    fileName = getFileDialog.selectedFiles().first();
                    if (!fileName.isEmpty() && !suffix.isEmpty())
                    {
                        suffix = suffix.split(QRegExp("[()*]"), Qt::SkipEmptyParts).last();
                        fileName += suffix;
                    }
                }
                break;

            case QFileDialog::Rejected:
            {
                return;
            }
            default:
                break;
            }

            std::thread saveImage(saveAsFile, image, fileName);
            saveImage.detach();

        };

        switch (sceneIndex)
        {
        case SceneIndex::None:
            break;
        case PreviewScene:
            {
                save(m_PreviewImageScene);
            }
            break;
        case GenerateLogoScene:
            {
                const auto logoScene = dynamic_cast<LifeSizeImageScene*>(m_AddLogoScene);
                logoScene->saveSceneAsImage(save);
            }
            break;
        }
    }

    void DisplayWidget::InitConnect()
    {
        connect(this, &DisplayWidget::sigOpen, this, [this](const std::string& path)
        {
            open(path);
        }, Qt::QueuedConnection);


        connect(this, &DisplayWidget::sigPreViewImage, [this](const std::string& filePath)
        {
            preViewImage(filePath);
        });

        connect(this, &DisplayWidget::sigShowImage, [this](size_t code)
        {
            const auto scene = dynamic_cast<PreViewImageScene*>(m_PreviewImageScene);
            emit scene->sigShowImage(code);
        });

    }
} // CM
