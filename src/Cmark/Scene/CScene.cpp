#include "CMark.h"

#include "CScene.h"

#include "PreViewImageItem.h"
#include "SceneDef.h"

#include <QFileDialog>
#include <QGraphicsView>

#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QTextCursor>

namespace CM
{
#ifdef TOOLWIDGET
    namespace
    {
        QGraphicsProxyWidget *CreateItem(const QSizeF &minimum = QSizeF(100.0, 100.0),
                                         const QSizeF &preferred = QSize(150.0, 100.0),
                                         const QSizeF &maximum = QSizeF(200.0, 100.0),
                                         const QString &name = "0")
        {
            auto w = new QGraphicsProxyWidget;
            w->setContentsMargins(0, 0, 0, 0);
            w->setWindowFrameMargins(0, 0, 0, 0);
            auto p = new QPushButton(name);
            w->setWidget(p);
            w->setData(0, name);
            w->setMinimumSize(minimum);
            w->setPreferredSize(preferred);
            w->setMaximumSize(maximum);

            w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            return w;
        }
    }
#endif

    CScene::CScene(QObject *parent)
        : QGraphicsScene(parent), m_LogoItem(new QGraphicsPixmapItem), m_SceneLayout(std::make_shared<SceneLayoutSettings>())
    {
        m_ShowImageItem = new PreViewImageItem(nullptr, m_SceneLayout);
        Init();
        {
            const auto &[FixImageSizeW, FixImageSizeH] = SceneLayoutSettings::fixPreViewImageSize();
            const auto newWidth = FixImageSizeW;
            const auto newHeight = FixImageSizeH;
            m_SceneLayout->setImageSize({newWidth, newHeight});
            m_SceneLayout->update();
        }
    }

    CScene::~CScene() = default;

    void CScene::Init()
    {
        m_PlainTextFont.setFamily("Microsoft YaHei");
        // m_PlainTextFont.setPointSize(11);
        m_PlainTextFont.setPixelSize(20);

        InitMargin();
        InitTargetImageItem();
        InitTexItems();
        InitLogoItem();
        InitSplitRect();

        connect(this, &CScene::sigNoScenes, [this](const bool val)
                {
            m_SplitRectItem->setVisible(val);
            m_LogoItem->setVisible(val);
            m_Bottom->setVisible(val);

            for (const auto& [key, it] : m_TextItem)
            {
                it->setVisible(val);
            } });

#ifdef TOOLWIDGET
        constexpr QSizeF minSize(30, 100);
        constexpr QSizeF prefSize(210, 100);
        constexpr QSizeF maxSize(300, 100);

        auto a = CreateItem(minSize, prefSize, maxSize, "A");
        {
            auto ap = dynamic_cast<QPushButton *>(a->widget());
            ap->setText("Camera Maker");
            ap->setContentsMargins(0.0, 0.0, 0.0, 0.0);
            ap->setStyleSheet("border: none;");
            ap->setStyleSheet("QPushButton{text-align : left;}");
            ap->setFlat(true);

            auto pa = ap->palette();
            pa.setColor(QPalette::Background, QColor(255, 255, 255, 255));
            ap->setPalette(pa);
        }

        auto b = CreateItem(minSize, prefSize, maxSize, "B");

        {
            auto bp = dynamic_cast<QPushButton *>(b->widget());
            bp->setText("Photo Times");
            bp->setContentsMargins(0.0, 0, 0.0, 0.0);
            bp->setStyleSheet("border: none;");
            bp->setStyleSheet("QPushButton{text-align : left;}");
            bp->setFlat(true);

            auto pa = bp->palette();
            pa.setColor(QPalette::Background, QColor(255, 255, 255, 255));
            bp->setPalette(pa);
        }

        auto c = CreateItem(minSize, prefSize, maxSize, "C");
        dynamic_cast<QPushButton *>(c->widget())->setText("Lens Group");

        auto d = CreateItem(minSize, prefSize, maxSize, "D");
        dynamic_cast<QPushButton *>(d->widget())->setText("Lens Infos");

        const auto toolWidgetLayout = new QGraphicsAnchorLayout;
        toolWidgetLayout->setSpacing(0);
        toolWidgetLayout->setContentsMargins(0, 0, 0, 0);

        a->setMaximumHeight(20);
        a->setMaximumWidth(200);
        a->setContentsMargins(0, 0, 0, 0);

        b->setMaximumHeight(20);
        b->setMaximumWidth(200);

        toolWidgetLayout->addAnchor(a, Qt::AnchorTop, toolWidgetLayout, Qt::AnchorTop);
        toolWidgetLayout->addAnchor(a, Qt::AnchorLeft, toolWidgetLayout, Qt::AnchorLeft);

        toolWidgetLayout->addAnchor(b, Qt::AnchorLeft, toolWidgetLayout, Qt::AnchorLeft);
        const auto anchor = toolWidgetLayout->addAnchor(b, Qt::AnchorTop, a, Qt::AnchorBottom);
        anchor->setSpacing(24);
        toolWidgetLayout->addAnchor(b, Qt::AnchorRight, a, Qt::AnchorRight);

        toolWidgetLayout->setHorizontalSpacing(0);
        toolWidgetLayout->setVerticalSpacing(0);

        m_ToolWidget = new QGraphicsWidget(nullptr, Qt::Window | Qt::FramelessWindowHint);

        m_ToolWidget->setLayout(toolWidgetLayout);
        m_ToolWidget->setPos(30, 200);
        m_ToolWidget->setContentsMargins(0, 0, 0, 0);
        m_ToolWidget->setWindowFrameMargins(0, 0, 0, 0);
        // this->addItem(m_ToolWidget);
#endif
    }

    void CScene::InitTexItems()
    {
        /// default show infos in image
        m_ShowInfos.emplace_back(ShowExifInfo{ShowExifTexPositionIndex::LeftTop, {ExifKey::CameraModel}});
        m_ShowInfos.emplace_back(ShowExifInfo{ShowExifTexPositionIndex::LeftBottom, {ExifKey::ImageDate}});
        m_ShowInfos.emplace_back(ShowExifInfo{ShowExifTexPositionIndex::RightTop, {ExifKey::LensModel}});
        m_ShowInfos.emplace_back(ShowExifInfo{
            ShowExifTexPositionIndex::RightBottom,
            {ExifKey::FocalLength, ExifKey::FStop, ExifKey::ExposureTime, ExifKey::ISOSpeed}});

        m_TextItem.insert({ShowExifTexPositionIndex::LeftTop, addText("")});
        m_TextItem.insert({ShowExifTexPositionIndex::LeftBottom, addText("")});
        m_TextItem.insert({ShowExifTexPositionIndex::RightTop, addText("")});
        m_TextItem.insert({ShowExifTexPositionIndex::RightBottom, addText("")});

        QFont font;
        font.setFamily("Microsoft YaHei");
        font.setPointSize(11);
        font.setPixelSize(13);

        for (auto &[key, item] : m_TextItem)
        {
            switch (key)
            {
            case ShowExifTexPositionIndex::LeftTop:
            case ShowExifTexPositionIndex::RightTop:
                font.setBold(true);
                item->setFont(font);
                font.setBold(false);
                break;
            case ShowExifTexPositionIndex::LeftBottom:
            case ShowExifTexPositionIndex::RightBottom:
                item->setFont(font);
                break;
            }
        }
    }

    void CScene::resetTexItemsPlainText(const ExifInfoMap &exifInfoMap)
    {
        m_TargetImageExifInfoLists = exifInfoMap; ///< copy infos

        for (const auto &[layoutIndex, keys] : m_ShowInfos)
        {
            const auto item = m_TextItem.at(layoutIndex);
            std::vector<std::string> exifInfos;
            std::for_each(keys.begin(), keys.end(), [&](const ExifKey &key)
                          { exifInfos.emplace_back(m_TargetImageExifInfoLists[key]); });

            auto res = std::accumulate(exifInfos.begin(), exifInfos.end(), std::string(),
                                       [](const std::string &a, const std::string &b)
                                       {
                                           if (a.empty() && b.empty())
                                           {
                                               return std::string();
                                           }
                                           const auto tail = b.empty() ? std::string() : std::string(" ") + b;
                                           return a + tail;
                                       });
            item->setVisible(!res.empty());
            item->setPlainText(res.c_str());
        }

        /// TODO: may be applyLayout it
        const auto rightTopTextRect = m_TextItem.at(ShowExifTexPositionIndex::RightTop)->boundingRect();
        const auto rightBottomTextRect = m_TextItem.at(ShowExifTexPositionIndex::RightBottom)->boundingRect();
        auto maxW = rightTopTextRect.width() > rightBottomTextRect.width()
                        ? rightTopTextRect.width()
                        : rightBottomTextRect.width();

        if (maxW < 200)
            maxW = 200;
        m_SceneLayout->setRightMaxWidth(static_cast<int>(maxW));

        updateTexItemsPosition();
    }

    void CScene::updateTexItemsPosition()
    {
        auto &[left, right, top, bottom] = m_SceneLayout->getMargin();
        const auto logoWithImageSpacing = m_SceneLayout->logoWithImageSpace();
        const auto &logoSize = m_SceneLayout->logoSize();
        const auto &imageSize = m_SceneLayout->imageSize();

        auto alignment = [](QGraphicsTextItem *item, Qt::AlignmentFlag f)
        {
            // QTextBlockFormat format;
            // format.setAlignment(f);
            // QTextCursor cursor = item->textCursor();
            // cursor.select(QTextCursor::Document);
            // cursor.mergeBlockFormat(format);
            // cursor.clearSelection();
            // item->setTextCursor(cursor);
        };

        for (const auto &[layoutIndex, keys] : m_ShowInfos)
        {
            const auto item = m_TextItem.at(layoutIndex);
            const auto &itemRect = item->boundingRect().toRect();

            switch (layoutIndex)
            {
            case ShowExifTexPositionIndex::LeftTop:
            {
                QPoint position(m_SceneLayout->leftTextOffset(), top + imageSize.h + logoWithImageSpacing);
                item->setPos(position);
                alignment(item, Qt::AlignLeft);
            }
            break;
            case ShowExifTexPositionIndex::LeftBottom:
            {
                QPoint position(m_SceneLayout->leftTextOffset(),
                                top + logoWithImageSpacing + imageSize.h + logoSize.h - itemRect.height());
                item->setPos(position);
                alignment(item, Qt::AlignLeft);
            }
            break;
            case ShowExifTexPositionIndex::RightTop:
            {
                QPoint position(
                    left + imageSize.w + right - m_SceneLayout->rightTextMaxWidth() - m_SceneLayout->rightTextOffset(), top + imageSize.h + logoWithImageSpacing);
                item->setPos(position);
                alignment(item, Qt::AlignRight);
            }
            break;
            case ShowExifTexPositionIndex::RightBottom:
            {
                QPoint position(
                    left + imageSize.w + right - m_SceneLayout->rightTextMaxWidth() - m_SceneLayout->rightTextOffset(), top + logoWithImageSpacing + imageSize.h + logoSize.h - itemRect.height());
                item->setPos(position);
                alignment(item, Qt::AlignRight);
            }
            break;
            }
        }
    }

    void CScene::InitLogoItem()
    {
        auto pixmap = QPixmap();
        pixmap.fill(Qt::TransparentMode);
        m_LogoItem->setPixmap(pixmap);
        addItem(m_LogoItem);
    }

    void CScene::resetLogoPixmap(const std::shared_ptr<QPixmap> &logo, CameraIndex cameraIndex)
    {
        const auto &[logoW, logoH] = m_SceneLayout->logoSize();

        const auto h = static_cast<float>(logoH);
        const auto ratio = static_cast<float>(logo->height()) / static_cast<float>(logo->width());
        auto newW = static_cast<int>(std::round(h / ratio));

        m_SceneLayout->setLogoSize(newW, logoH);
        m_LogoItem->setPixmap(logo->scaled({newW, logoH}, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_LogoItem->setData(static_cast<int>(GraphicsItemDataIndex::CameraIndex), static_cast<int>(cameraIndex));
        m_LogoItem->setVisible(cameraIndex != CameraIndex::None);

        updateLogoPosition();
    }

    void CScene::updateLogoPosition()
    {
        const auto logoSpaceWithImage = m_SceneLayout->logoWithImageSpace();
        const auto [imageW, imageH] = m_SceneLayout->imageSize();
        const auto &[l, r, t, b] = m_SceneLayout->getMargin();

        const auto x = l + r + imageW - m_SceneLayout->logoWithSplitLineSpace() * 2.0 - m_SceneLayout->logoSize().w -
                       m_SceneLayout->rightTextMaxWidth() - m_SceneLayout->rightTextOffset() - m_SceneLayout->splitRectWidth();
        const auto y = t + imageH + logoSpaceWithImage;
        m_LogoItem->setPos(x, y);
    }

    void CScene::InitTargetImageItem()
    {
        auto pixmap = QPixmap();
        pixmap.fill(Qt::transparent);
        m_ShowImageItem->setPixmap(pixmap);
        addItem(m_ShowImageItem);
    }

    void CScene::resetPreviewImageTarget(const QPixmap &pixmap, size_t imageIndexCode)
    {
        m_SceneLayout->setImageSize({pixmap.width(), pixmap.height()});
        m_ShowImageItem->resetPixmap(imageIndexCode);
        m_ShowImageItem->setData(static_cast<int>(GraphicsItemDataIndex::PixmapIndex), QVariant::fromValue(static_cast<qulonglong>(imageIndexCode)));

        m_ShowItemFlags = m_ShowImageItem->validImage();
    }

    void CScene::InitMargin()
    {
        const auto rect = sceneRect().toRect();
        const auto &[left, right, top, bottom] = m_SceneLayout->getMargin();

        m_Left = new QGraphicsRectItem(0, 0, left, rect.width());
        m_Right = new QGraphicsRectItem(0, 0, right, rect.height());
        m_Top = new QGraphicsRectItem(0, 0, rect.width(), top);
        m_Bottom = new QGraphicsRectItem(0, 0, m_SceneLayout->getMargin().m_Left, rect.width());

        const auto &rectPen = QPen(Qt::transparent);
        m_Left->setPen(rectPen);
        m_Right->setPen(rectPen);
        m_Top->setPen(rectPen);
        m_Bottom->setPen(rectPen);

        addItem(m_Left);
        addItem(m_Right);
        addItem(m_Top);
        addItem(m_Bottom);
    }

    void CScene::updateMarginItems()
    {
        const auto &[left, right, top, bottom] = m_SceneLayout->getMargin();
        const auto logoSpaceWithShowImage = m_SceneLayout->logoWithImageSpace();
        const auto &imageRect = m_SceneLayout->imageSize();
        const auto &logoRect = m_SceneLayout->logoSize();

        const auto sceneBoundMarginRectH = top + imageRect.h + logoSpaceWithShowImage + logoRect.h + bottom;
        const auto sceneBoundMarginRectW = left + imageRect.w + right;

        m_Left->setPen(QPen(Qt::NoPen));
        m_Right->setPen(QPen(Qt::NoPen));
        m_Top->setPen(QPen(Qt::NoPen));
        m_Bottom->setPen(QPen(Qt::NoPen));

        m_Left->setRect(0, 0, left, sceneBoundMarginRectH);
        m_Right->setRect(left + imageRect.w, 0, right, sceneBoundMarginRectH);
        m_Top->setRect(0, 0, sceneBoundMarginRectW, top);
        m_Bottom->setRect(0, top + imageRect.h + logoSpaceWithShowImage + logoRect.h, sceneBoundMarginRectW, bottom);

        m_Left->setVisible(0 != left);
        m_Right->setVisible(0 != right);
        m_Top->setVisible(0 != top);
    }

    void CScene::InitSplitRect()
    {
        m_SplitRectItem = new QGraphicsRectItem;
        QPen pen;
        pen.setWidth(1);
        pen.setColor(Qt::transparent);

        QBrush brush(Qt::SolidPattern);
        brush.setColor(QColor(219, 219, 219));

        QRadialGradient radialGradient(100, 100, 100, 0, 100);
        radialGradient.setColorAt(0, QColor(253, 253, 253));
        radialGradient.setColorAt(0.1, QColor(253, 253, 253));
        radialGradient.setColorAt(0.2, QColor(216, 216, 216));
        radialGradient.setColorAt(0.5, QColor(218, 218, 218));
        radialGradient.setColorAt(0.6, QColor(217, 217, 217));
        radialGradient.setColorAt(0.9, QColor(222, 222, 222));
        radialGradient.setColorAt(1.0, QColor(241, 241, 241));

        m_SplitRectItem->setBrush(radialGradient);
        m_SplitRectItem->setPen(Qt::NoPen);

        addItem(m_SplitRectItem);
    }

    void CScene::updateSplitRect()
    {
        const auto &logoSize = m_SceneLayout->logoSize();
        const auto imageH = m_SceneLayout->imageSize().h;
        const auto imageW = m_SceneLayout->imageSize().w;
        const auto spacing = m_SceneLayout->logoWithImageSpace();
        const auto &[left, right, top, bottom] = m_SceneLayout->getMargin();

        const auto splitRectW = m_SceneLayout->splitRectWidth();

        const auto x = left + right + imageW - m_SceneLayout->rightTextOffset() - m_SceneLayout->rightTextMaxWidth() -
                       m_SceneLayout->logoWithSplitLineSpace() - splitRectW;
        const auto y = top + imageH + spacing;

        m_SplitRectItem->setRect(x, y, splitRectW, logoSize.h);

        const auto r = m_SplitRectItem->rect();

        QLinearGradient radialGradient(r.left(), r.top(), r.right(), r.top());
        radialGradient.setColorAt(0, QColor(253, 253, 253));
        radialGradient.setColorAt(0.2, QColor(216, 216, 216));
        radialGradient.setColorAt(0.5, QColor(218, 218, 218));
        radialGradient.setColorAt(0.6, QColor(217, 217, 217));
        radialGradient.setColorAt(0.9, QColor(222, 222, 222));
        radialGradient.setColorAt(1.0, QColor(241, 241, 241));
        m_SplitRectItem->setBrush(radialGradient);

        /// make rect visiable == false
        m_SplitRectItem->setVisible(m_LogoItem->isVisible());
    }

    void CScene::applyLayout(const std::shared_ptr<SceneLayoutSettings> &layout)
    {
        if (layout)
        {
            *m_SceneLayout = *layout;
        }
        updateShowImage();
        updateTexItemsPosition();
        updateLogoPosition();
        updateSplitRect();
        updateMarginItems();

        if (!m_ShowItemFlags)
        {
            emit sigNoScenes(false);
        }

#ifdef TOOLWIDGET
        auto logoPos = m_LogoItem->pos();
        auto x = m_SceneLayout->leftTextOffset();
        m_ToolWidget->setPos(x, logoPos.y());
#endif
    }

    void CScene::updateLayout()
    {
        m_SceneLayout->update();
    }

    std::shared_ptr<SceneLayoutSettings> CScene::layoutSettings() const
    {
        return m_SceneLayout;
    }

    void CScene::resetStatus()
    {
        m_ShowItemFlags = true;
    }

    void CScene::updateShowImage()
    {
        m_ShowImageItem->applyLayout(m_SceneLayout);
    }
} // CM
