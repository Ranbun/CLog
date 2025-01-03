#include "ImageProcess.h"

#include <Log/CLog.h>

#include <QBuffer>
#include <QDateTime>

namespace
{
    [[maybe_unused]] auto ImageSaveDefaultName() -> QString
    {
        const QDateTime currentDateTime = QDateTime::currentDateTime();
        auto outputName = currentDateTime.toString("yyyy-MM-dd__HHHmmMssS");

        constexpr std::hash<std::string> nameGenerator;
        const auto nameCode = nameGenerator(outputName.toStdString());
        outputName = outputName + "__" + QString::number(nameCode);
        return {outputName};
    }
}

namespace CM
{
    std::shared_ptr<QByteArray> ImageProcess::loadFile(const QString &fileName)
    {
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly))
        {
            return std::make_shared<QByteArray>();
        }

        auto data = std::make_shared<QByteArray>(file.readAll());
        file.close();
        return data;
    }

    std::shared_ptr<QImage> ImageProcess::toQImage(const std::shared_ptr<QByteArray> &imageData, const QString &format)
    {
        QBuffer readAsImageBuffer(imageData.get());
        {
            readAsImageBuffer.open(QIODevice::ReadOnly);
            readAsImageBuffer.seek(0);
        }

        auto imageReader = std::make_shared<QImageReader>(&readAsImageBuffer, format.toStdString().c_str());
        imageReader->setAutoTransform(true);
        auto im = std::make_shared<QImage>(imageReader->read());
        readAsImageBuffer.close();
        readAsImageBuffer.deleteLater();
        imageReader.reset();

        return im;
    }

    std::shared_ptr<QImage> ImageProcess::loadImage(const QString &fileName)
    {
        const QFileInfo fileInfo(fileName);

        const auto format = fileInfo.suffix().toLower();
        const auto data = loadFile(fileInfo.filePath());
        return toQImage(data, format);
    }

    void ImageProcess::save(const std::shared_ptr<QPixmap> &pixmap, const QString &fileName)
    {
        auto name = fileName;
        if (name.isEmpty())
        {
            name = ImageSaveDefaultName() + ".png";
        }

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);

        const QFileInfo fileInfo(name);
        if (fileInfo.exists())
        {
            buffer.close();
            CLogInstance.PrintMes<QString>(name + QString(" exists."));
            return;
        }
        const auto format = fileInfo.suffix().toLower().toStdString();

        if (pixmap->save(&buffer, format.c_str()))
        {
            if (QFile file(name);
                file.open(QIODevice::WriteOnly))
            {
                file.write(buffer.data());
                file.close();
                CLogInstance.PrintMes<QString>(QString("Image saved successfully."));
            }
        }

        buffer.close();
    }

    void ImageProcess::save(const std::shared_ptr<QImage> &image, const QString &fileName)
    {
        auto name = fileName;
        if (name.isEmpty())
        {
            name = ImageSaveDefaultName() + ".png";
        }

        if (QFile file(name);
            file.open(QIODevice::WriteOnly))
        {
            const QFileInfo fileInfo(name);
            const auto format = fileInfo.suffix().toLower().toStdString();

            if (QImageWriter imageWriter(&file, format.c_str());
                imageWriter.write(*image))
            {
                CLog::Info(QString("Image saved successfully."));
            }

            file.close();
        }
    }

    float ImageProcess::imageRatio(const QPixmap &pixmap)
    {
        const auto size = QSizeF(pixmap.size());
        return size.width() / size.height(); // NOLINT(clang-diagnostic-implicit-float-conversion)
    }

} // CM
