#include "BatchImageProcess.h"

#include <QDir>
#include <QPainter>
#include <SceneLayoutSettings.h>
#include <utility>
#include <CThread/ThreadPool.h>
#include <File/LogoManager.h>
#include <File/ResourcesTool.h>
#include <File/ImageProcess/ImageProcess.h>
#include <File/Resolver/EXIFResolver.h>
#include <Log/CLog.h>
#include <Scene/CScene.h>

namespace CM
{
    namespace
    {
        std::mutex g_FileInfosMutex;
        std::map<size_t, QFileInfo> g_LoadedFileInfos;

        /**
         * @brief 保存图片时候传递的信息
         */
        struct WritePack
        {
            size_t m_FileIndexCode;
            QString m_FileName{""};
            [[maybe_unused]] int m_W;
            [[maybe_unused]] int m_H;
        };

        /**
         * @brief 加载图片(添加到管理器 & 解析信息)
         * @param fileInfo 图片文件路径
         */
        void LoadFile(const QFileInfo& fileInfo)
        {
            if (!fileInfo.exists())
            {
                CLog::Info(QString("File: ") + fileInfo.filePath() + QString(" exists!"));
                return;
            }

            const auto& [w, h] = SceneLayoutSettings::fixPreViewImageSize();
            const auto fileIndexCode = ImageProcess::generateFileIndexCode(fileInfo.filePath().toStdString());
            const auto data = ImageProcess::loadFile(fileInfo.filePath());
            const ImagePack loadImagePack{
                fileIndexCode, data, fileInfo.filePath().toStdString(), std::make_shared<std::mutex>(), {w, h}
            };
            /// load image
            PictureManager::loadImage(loadImagePack);
            EXIFResolver::resolver(loadImagePack);

            data->clear();

            {
                std::lock_guard local(g_FileInfosMutex);
                g_LoadedFileInfos.insert({fileIndexCode, fileInfo}); /// TODO: danger
            }

            {
                CLog::Info(QString("File: ") + fileInfo.filePath() + QString(" load success!"));
            }
        }

        /**
         * @brief 写图片到磁盘时候传递的信息
         * @param pack 传递的信息
         */
        void WriteFile(const WritePack& pack)
        {
            const auto cameraLogoIndex = LogoManager::resolverCameraIndex(
                EXIFResolver::ExifItem(pack.m_FileIndexCode, ExifKey::CameraMake));
            const auto logo = LogoManager::getCameraMakerLogo(cameraLogoIndex);

            const auto preViewImage = PictureManager::getImage(pack.m_FileIndexCode);

            SceneLayoutSettings layout;
            auto size = preViewImage->size();
            layout.setImageSize({size.width(), size.height()});
            layout.update();

            auto newH = size.height() + layout.logoWithImageSpace() + layout.logoSize().h + layout.getMargin().m_Bottom + layout.getMargin().m_Top;
            auto newW = size.width() + layout.getMargin().m_Left + layout.getMargin().m_Right;
            const auto output = std::make_shared<QPixmap>(newW, newH);

            output->fill(Qt::white);

            /// 计算绘制的字体& 绘制的图标的位置
            size = output->size();
            const auto x = size.width() - layout.logoWithSplitLineSpace() * 2 - layout.logoSize().w -
                layout.rightTextMaxWidth() - layout.rightTextOffset() - layout.splitRectWidth();
            const auto y = layout.getMargin().m_Top + preViewImage->size().height() + layout.logoWithImageSpace();

            QPainter pen(&(*output));

            auto& [logoW, logoH] = layout.logoSize();

            pen.drawPixmap(0, 0, *preViewImage);

            /// draw logo
            pen.drawPixmap(x, y, logo->scaled({logoW, logoH}, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            /// 绘制文字
            const auto exifInfosmap = EXIFResolver::getExifInfo(pack.m_FileIndexCode);
            struct outputLayoutInfo
            {
                ShowExifTexPositionIndex m_LayoutIndex;
                std::string m_Infos;
            };
            std::vector<ShowExifInfo> showInfos;
            std::vector<outputLayoutInfo> outputLayoutInfos;
            {
                showInfos.emplace_back(ShowExifInfo{ShowExifTexPositionIndex::LeftTop, {ExifKey::CameraModel}});
                showInfos.emplace_back(ShowExifInfo{ShowExifTexPositionIndex::LeftBottom, {ExifKey::ImageDate}});
                showInfos.emplace_back(ShowExifInfo{ShowExifTexPositionIndex::RightTop, {ExifKey::LensModel}});
                showInfos.emplace_back(ShowExifInfo{
                    ShowExifTexPositionIndex::RightBottom,
                    {ExifKey::FocalLength, ExifKey::FStop, ExifKey::ExposureTime, ExifKey::ISOSpeed}
                });

                for (const auto& [layoutIndex, keys] : showInfos)
                {
                    if (keys.empty())
                    {
                        outputLayoutInfos.emplace_back(outputLayoutInfo{layoutIndex, ""});
                        continue;
                    }

                    std::vector<std::string> exifInfos;
                    std::for_each(keys.begin(), keys.end(), [&](const ExifKey& key)
                    {
                        if (exifInfosmap.count(key))
                        {
                            exifInfos.emplace_back(exifInfosmap.at(key));
                        }
                    });

                    auto res = std::accumulate(exifInfos.begin(), exifInfos.end(), std::string(),
                                               [](const std::string& a, const std::string& b)
                                               {
                                                   if (a.empty() && b.empty())
                                                   {
                                                       return std::string();
                                                   }
                                                   const auto tail = b.empty() ? std::string() : b;
                                                   auto temp = a;
                                                   if (!b.empty() && !a.empty())
                                                   {
                                                       temp += " ";
                                                   }
                                                   return temp + tail;
                                               });

                    outputLayoutInfos.emplace_back(outputLayoutInfo{layoutIndex, std::move(res)});
                }
            }

            const auto& [l, r, t, b] = layout.getMargin();
            auto [layoutImageSizeW, layoutImageSizeH] = layout.imageSize();

            QTextOption textOption;
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop); // 居中对齐
            pen.setPen(Qt::black);

            for (auto [index, infos] : outputLayoutInfos)
            {
                QFont font;
                font.setFamily("Microsoft YaHei");
                font.setPointSize(11);
                font.setPixelSize(13);

                switch (index)
                {
                case ShowExifTexPositionIndex::LeftTop:
                    {
                        font.setBold(true);
                        pen.setFont(font);
                        /// 计算位置
                        QFontMetrics metrics(pen.font());
                        [[maybe_unused]] int textWidth = metrics.horizontalAdvance(infos.c_str());
                        int textHeight = metrics.height();

                        QPoint position(layout.leftTextOffset(),
                                        t + preViewImage->size().height() + layout.logoWithImageSpace() + textHeight);
                        pen.drawText(position, infos.c_str());
                    }
                    break;
                case ShowExifTexPositionIndex::LeftBottom:
                    {
                        pen.setFont(font);
                        // 计算位置
                        QFontMetrics metrics(pen.font());
                        int textWidth = metrics.horizontalAdvance(infos.c_str());
                        int textHeight = metrics.height();
                        QPoint position(layout.leftTextOffset(),
                                        t + preViewImage->size().height() + layout.logoWithImageSpace() +
                                        layout.logoSize().h - textHeight);

                        auto rect = QRect(position, QSize{textWidth, textHeight});
                        pen.fillRect(rect, Qt::white);
                        pen.drawText(rect, infos.c_str(), textOption);
                    }
                    break;
                case ShowExifTexPositionIndex::RightTop:
                    {
                        font.setBold(true);
                        pen.setFont(font);
                        QFontMetrics metrics(pen.font());
                        [[maybe_unused]] int textWidth = metrics.horizontalAdvance(infos.c_str());
                        int textHeight = metrics.height();
                        QPoint position(
                            l + layoutImageSizeW + r - layout.rightTextMaxWidth() - layout.rightTextOffset(),
                            t + layoutImageSizeH + layout.logoWithImageSpace() + textHeight);

                        pen.drawText(position, infos.c_str());
                    }
                    break;
                case ShowExifTexPositionIndex::RightBottom:
                    {
                        pen.setFont(font);
                        // 计算位置
                        QFontMetrics metrics(pen.font());
                        int textWidth = metrics.horizontalAdvance(infos.c_str());
                        int textHeight = metrics.height();
                        QPoint position(l + layoutImageSizeW + r - layout.rightTextMaxWidth() - layout.rightTextOffset(),
                                        t + preViewImage->size().height() + layout.logoWithImageSpace() +
                                        layout.logoSize().h - textHeight);

                        auto rect = QRect(position, QSize{textWidth, textHeight});
                        pen.fillRect(rect, Qt::white);
                        pen.drawText(rect, infos.c_str(), textOption);
                    }
                    break;
                }
            }

            pen.end();

            /// 保存图片
            ImageProcess::save(output, pack.m_FileName);
        }
    }


    BatchImageProcessor::BatchImageProcessor(QString rootPath)
        : m_Root(std::move(rootPath))
        , m_AvailableFileType({"jpeg", "jpg"})
        , m_Pool(new ThreadPool(5))

    {
    }

    BatchImageProcessor::~BatchImageProcessor()
    {
        delete m_Pool;
        m_Pool = nullptr;
    }

    QFileInfoList& BatchImageProcessor::availableFileInfos()
    {
        m_ImageFileLists.clear();
        scanDirectory(m_Root);
        return m_ImageFileLists;
    }

    void BatchImageProcessor::scanDirectory(const QString& path)
    {
        /// TODO: 此处扫描没有设置上限, 所以可能会爆炸

        const QDir dir(path);
        // 判断目录是否存在
        if (!dir.exists())
        {
            CLog::Warning(QString("Directory does not exist: "));
            return;
        }

        // 获取目录下的所有文件和子文件夹
        QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        // 遍历文件和子文件夹
        foreach(const QString & entry, entries)
        {
            QString entryPath = dir.filePath(entry);

            // 如果是文件夹，则递归扫描
            if (QFileInfo(entryPath).isDir() && !m_ScannedDirectories.contains(entryPath))
            {
                m_ScannedDirectories.insert(entryPath);
                scanDirectory(entryPath);
            }
            else
            {
                auto fileInfo = QFileInfo(entryPath);
                const auto fileType = fileInfo.suffix().toLower();
                if (m_AvailableFileType.contains(fileType))
                {
                    m_ImageFileLists.append(fileInfo);
                }
            }
        }
    }

    void BatchImageProcessor::Run()
    {
        auto imageFileLists = availableFileInfos();

        std::vector<std::future<void>> futures;
        for (auto& fileInfo : imageFileLists)
        {
            futures.emplace_back(m_Pool->enqueue(LoadFile, std::ref(fileInfo)));
        }

        for (auto& future : futures)
        {
            future.wait(); /// wait thread
        }


        futures.clear();

        for (auto& [fileIndexCode, pixmap] : PictureManager::images())
        {
            if (!g_LoadedFileInfos.count(fileIndexCode))
            {
                continue;
            }

            const auto& [w, h] = SceneLayoutSettings::fixPreViewImageSize();
            const auto fileName = g_LoadedFileInfos.at(fileIndexCode).fileName();
            futures.emplace_back(m_Pool->enqueue(WriteFile, WritePack{fileIndexCode, fileName, w, h}));
        }

        /// wait thread finish
        for (auto& future : futures)
        {
            future.wait();
        }
    }
}
