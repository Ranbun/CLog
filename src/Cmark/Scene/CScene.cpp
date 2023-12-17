#include "CMark.h"

#include "CScene.h"
#include "PreViewImageItem.h"

#include <QGraphicsView>
#include <QFileDialog>

namespace CM
{
    CScene::CScene(QObject *parent)
            : QGraphicsScene(parent)
            , m_showImageItem(new PreViewImageItem(nullptr, m_sceneLayout))
            , m_logoItem(new QGraphicsPixmapItem)
            , m_splitRectItem(nullptr)
    {
        Init();
    }

    void CScene::Init()
    {
        InitMargin();
        InitTargetImageItem();
        InitTexItems();
        InitLogoItem();
        InitSplitRect();
    }

    void CScene::InitTexItems()
    {
        /// default show infos in image
        m_showInfos.emplace_back(showExifInfo{showExifTexPositionIndex::left_top, {ExifKey::Camera_model}});
        m_showInfos.emplace_back(showExifInfo{showExifTexPositionIndex::left_bottom, {ExifKey::Image_date}});
        m_showInfos.emplace_back(showExifInfo{showExifTexPositionIndex::right_top, {ExifKey::Lens_Model}});
        m_showInfos.emplace_back(showExifInfo{showExifTexPositionIndex::right_bottom, {ExifKey::FocalLength,ExifKey::F_stop, ExifKey::Exposure_time, ExifKey::ISO_speed}});   ///< TODO： 可以显示多条信息 需要做 || 处理

        m_textItem.insert({showExifTexPositionIndex::left_top, addText("")});
        m_textItem.insert({showExifTexPositionIndex::left_bottom, addText("")});
        m_textItem.insert({showExifTexPositionIndex::right_top, addText("")});
        m_textItem.insert({showExifTexPositionIndex::right_bottom, addText("")});

        QFont font;
        font.setFamily("Microsoft YaHei");
        font.setPointSize(11);
        font.setPixelSize(13);

        for(auto & [key,item]: m_textItem)
        {
            switch (key)
            {
                case showExifTexPositionIndex::left_top:
                case showExifTexPositionIndex::right_top:
                    font.setBold(true);
                    item->setFont(font);
                    font.setBold(false);
                    break;
                default:
                    item->setFont(font);
                    break;
            }

        }
    }

    void CScene::resetTexItemsPlainText(const ExifInfoMap &exifInfoMap)
    {
        m_targetImageExifInfoLists = exifInfoMap;  ///< copy infos

        for(const auto & [layoutIndex, keys]: m_showInfos)
        {
            const auto item = m_textItem.at(layoutIndex);
            std::vector<std::string> exifInfos;
            std::for_each(keys.begin(),keys.end(),[&](const ExifKey & key)
            {
                exifInfos.emplace_back(m_targetImageExifInfoLists[key]);
            });

            auto res = std::accumulate(exifInfos.begin(), exifInfos.end(), std::string(), [](const std::string& a, const std::string& b){

                auto tail = b.empty() ? std::string() : std::string(" ") + b;
                return a + tail;
            });

            item->setPlainText(res.c_str());
            const auto & itemRect = item->boundingRect().toRect();
        }

        /// TODO: may be applyLayout it
        auto rightTopTextRect = m_textItem.at(showExifTexPositionIndex::right_top)->boundingRect();
        auto rightBottomTextRect = m_textItem.at(showExifTexPositionIndex::right_bottom)->boundingRect();
        auto maxW = rightTopTextRect.width() > rightBottomTextRect.width() ? rightTopTextRect.width() : rightBottomTextRect.width();
        m_sceneLayout.setRightMaxWidth(static_cast<int>(maxW));

        updateTexItemsPosition();
    }

    void CScene::updateTexItemsPosition()
    {
        auto & [left,right,top,bottom] = m_sceneLayout.getMargin();
        auto logoWithImageSpacing = m_sceneLayout.logoWithImageSpace();
        const auto & logoSize = m_sceneLayout.LogoSize();
        const auto & imageSize = m_sceneLayout.ImageSize();

        for(const auto & [layoutIndex, keys]: m_showInfos)
        {
            const auto item = m_textItem.at(layoutIndex);
            const auto & itemRect = item->boundingRect().toRect();

            switch (layoutIndex)
            {
                case showExifTexPositionIndex::left_top:
                {
                    QPoint position(m_sceneLayout.leftTextOffset(), top + imageSize.height() + logoWithImageSpacing);
                    item->setPos(position);
                }
                    break;
                case showExifTexPositionIndex::left_bottom:
                {
                    QPoint position(m_sceneLayout.leftTextOffset(), top + logoWithImageSpacing + imageSize.height() + logoSize.h - itemRect.height());
                    item->setPos(position);
                }
                    break;
                case showExifTexPositionIndex::right_top:
                {
                    QPoint position(left + imageSize.width() + right - m_sceneLayout.rightTextMaxWidth() - m_sceneLayout.rightTextOffset(), top + imageSize.height() + logoWithImageSpacing);
                    item->setPos(position);
                }
                    break;
                case showExifTexPositionIndex::right_bottom:
                {
                    QPoint position(left + imageSize.width() + right - m_sceneLayout.rightTextMaxWidth() - m_sceneLayout.rightTextOffset(), top + logoWithImageSpacing + imageSize.height() + logoSize.h - itemRect.height());
                    item->setPos(position);
                }
                    break;
            }
        }
    }

    void CScene::InitLogoItem()
    {
        auto pixmap = QPixmap();
        pixmap.fill(Qt::TransparentMode);
        m_logoItem->setPixmap(pixmap);
        addItem(m_logoItem);
    }

    void CScene::resetLogoPixmap(std::shared_ptr<QPixmap> logo, CameraIndex cameraIndex)
    {
        const auto & logoSize = m_sceneLayout.LogoSize();

        auto h = static_cast<float>(logoSize.h);
        auto ratio = static_cast<float>(logo->height()) / static_cast<float>(logo->width());
        auto newW = (int)(std::round(h / ratio));

        m_sceneLayout.setLogoSize(newW,logoSize.height());
        m_logoItem->setPixmap(logo->scaled({newW,logoSize.height()}, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_logoItem->setData(static_cast<int>(GraphicsItemDataIndex::CameraIndex),cameraIndex);

        updateLogoPosition();
    }

    void CScene::updateLogoPosition()
    {
        auto logoSpaceWithImage = m_sceneLayout.logoWithImageSpace();
        auto imageSize = m_sceneLayout.ImageSize();
        const auto & [l,r,t,b] = m_sceneLayout.getMargin();

        auto x = l + r + imageSize.w - m_sceneLayout.logoWithSplitLineSpace() * 2.0 - m_sceneLayout.LogoSize().w -
                 m_sceneLayout.rightTextMaxWidth() - m_sceneLayout.rightTextOffset() - m_sceneLayout.splitRectWidth();
        auto y = t + imageSize.height() + logoSpaceWithImage;
        m_logoItem->setPos(x, y);
    }

    void CScene::InitTargetImageItem()
    {
        auto pixmap = QPixmap();
        pixmap.fill(Qt::transparent);
        m_showImageItem->setPixmap(pixmap);
        addItem(m_showImageItem);
    }

    void CScene::resetPreviewImageTarget(const QPixmap &pixmap)
    {
        if(auto item = dynamic_cast<CM::PreViewImageItem*>(m_showImageItem);
                item)
        {
            item->resetPixmap(pixmap);
        }
        m_sceneLayout.setImageSize({pixmap.width(),pixmap.height()});
    }

    void CScene::InitMargin()
    {
        auto rect = sceneRect().toRect();
        const auto & [left,right,top,bottom] = m_sceneLayout.getMargin();

        m_left = new QGraphicsRectItem(0,0,left,rect.width());
        m_right = new QGraphicsRectItem(0,0,right,rect.height());
        m_top = new QGraphicsRectItem(0,0,rect.width(),top);
        m_bottom = new QGraphicsRectItem(0,0,m_sceneLayout.getMargin().left,rect.width());

        const auto & rectPen = QPen(Qt::transparent);
        m_left->setPen(rectPen);
        m_right->setPen(rectPen);
        m_top->setPen(rectPen);
        m_bottom->setPen(rectPen);

        addItem(m_left);
        addItem(m_right);
        addItem(m_bottom);
        addItem(m_top);
    }

    void CScene::updateMarginItems()
    {
        const auto & [left,right,top,bottom] = m_sceneLayout.getMargin();
        auto logoSpaceWithShowImage = m_sceneLayout.logoWithImageSpace();
        const auto & imageRect = m_sceneLayout.ImageSize();
        const auto & logoRect = m_sceneLayout.LogoSize();

        auto sceneBoundMarginRectH = top + imageRect.height() + logoSpaceWithShowImage + logoRect.height() + bottom;
        auto sceneBoundMarginRectW = left + imageRect.width() + right;

        m_left->setRect(0, 0, left, sceneBoundMarginRectH);
        m_right->setRect(left + imageRect.width(), 0, right, sceneBoundMarginRectH);
        m_top->setRect(0,0,sceneBoundMarginRectW,top);

        m_bottom->setRect(0,top + imageRect.height() + logoSpaceWithShowImage + logoRect.height(),sceneBoundMarginRectW,bottom);
    }

    void CScene::InitSplitRect()
    {
        m_splitRectItem = new QGraphicsRectItem;
        QPen pen;
        pen.setWidth(1);
        pen.setColor(Qt::transparent);

        QBrush brush(Qt::SolidPattern);
        brush.setColor(QColor(219,219,219));

        QRadialGradient radialGradient(100, 100, 100, 0, 100);
        radialGradient.setColorAt(0, QColor(253,253,253));
        radialGradient.setColorAt(0.1, QColor(253,253,253));
        radialGradient.setColorAt(0.2, QColor(216,216,216));
        radialGradient.setColorAt(0.5, QColor(218,218,218));
        radialGradient.setColorAt(0.6, QColor(217,217,217));
        radialGradient.setColorAt(0.9, QColor(222,222,222));
        radialGradient.setColorAt(1.0, QColor(241,241,241));

        m_splitRectItem->setBrush(radialGradient);
        m_splitRectItem->setPen(Qt::NoPen);

        addItem(m_splitRectItem);
    }

    void CScene::updateSplitRect()
    {
        const auto & logoSize = m_sceneLayout.LogoSize();
        auto imageH = m_sceneLayout.ImageSize().h;
        auto imageW = m_sceneLayout.ImageSize().w;
        auto spacing = m_sceneLayout.logoWithImageSpace();
        const auto & [left,right,top,bottom] = m_sceneLayout.getMargin();

        auto splitRectW = m_sceneLayout.splitRectWidth();

        auto x = left + right + imageW - m_sceneLayout.rightTextOffset() - m_sceneLayout.rightTextMaxWidth() -
                m_sceneLayout.logoWithSplitLineSpace() - splitRectW;
        auto y = top + imageH + spacing;

        m_splitRectItem->setRect(x, y, splitRectW, logoSize.h);

        auto r = m_splitRectItem->rect();

        QLinearGradient radialGradient(r.left(),r.top(),r.right(),r.top());
        radialGradient.setColorAt(0, QColor(253,253,253));
        radialGradient.setColorAt(0.2, QColor(216,216,216));
        radialGradient.setColorAt(0.5, QColor(218,218,218));
        radialGradient.setColorAt(0.6, QColor(217,217,217));
        radialGradient.setColorAt(0.9, QColor(222,222,222));
        radialGradient.setColorAt(1.0, QColor(241,241,241));
        m_splitRectItem->setBrush(radialGradient);
    }

    void CScene::applyLayout()
    {
        updateShowImage();
        updateTexItemsPosition();
        updateLogoPosition();
        updateSplitRect();
        updateMarginItems();
    }

    void CScene::updateLayout()
    {
        m_sceneLayout.update();
    }

    void CScene::updateShowImage()
    {
        m_showImageItem->applyLayout();
    }

} // CM
