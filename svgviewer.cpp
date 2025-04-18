#include "svgviewer.h"
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <QFile>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextBrowser>
#include <QString>

SvgViewer::SvgViewer(QWidget* parent)
    : QGraphicsView(parent),
    scene(new QGraphicsScene(this)),
    backgroundItem(nullptr)
{
    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    // 禁用拖拽，保持视图固定
    setDragMode(QGraphicsView::NoDrag);
}

void SvgViewer::loadSvg(const QString &filePath) {
    // 清空场景
    scene->clear();
    backgroundItem = nullptr;

    // 使用 QGraphicsSvgItem 加载整个 SVG 图片
    backgroundItem = new QGraphicsSvgItem(filePath);
    if (!backgroundItem->renderer()->isValid()) {
        qWarning() << "❌ SVG 加载失败:" << filePath;
        delete backgroundItem;
        backgroundItem = nullptr;
        return;
    }
    // 保持图像可见
    backgroundItem->setOpacity(1.0);
    backgroundItem->setZValue(-1);  // 放在最底层
    scene->addItem(backgroundItem);

    // 设置场景区域为 SVG 背景图层的边界
    QRectF rect = backgroundItem->boundingRect();
    scene->setSceneRect(rect);

    // 重置变换，自动适配视图尺寸，并选择合适的缩放（初始放大2倍，可调整）
    resetTransform();
    fitInView(rect, Qt::KeepAspectRatio);
    scale(16.0, 16.0);
}

void SvgViewer::mousePressEvent(QMouseEvent* event) {
    QPointF scenePos = mapToScene(event->pos());
    QRectF rect = scene->sceneRect();
    
    QTextBrowser* browser = nullptr;
    if (parentWidget())
        browser = parentWidget()->findChild<QTextBrowser*>("textBrowser_log");

    if (!rect.isValid() || rect.isEmpty()) {
        if (browser)
            browser->append(QStringLiteral("❌ 无法获取背景区域信息！"));
        else
            qDebug() << "❌ 无法获取背景区域信息！";
    } else {
        double relativeX = (scenePos.x() - rect.x()) / rect.width();
        double relativeY = (scenePos.y() - rect.y()) / rect.height();
        int col = static_cast<int>(relativeX * 100);
        int row = static_cast<int>(relativeY * 50);
        {
            QString msg = QStringLiteral("映射到矩阵 (100x50) 中的坐标: (%1,%2)").arg(col).arg(row);
            if (browser)
                browser->append(msg);
            else
                qDebug() << msg;
        }

        // 加载 JSON 文件
        QFile file(":/map/resources/map.json");
        if (!file.open(QIODevice::ReadOnly)) {
            if (browser)
                browser->append(QStringLiteral("无法打开 JSON 文件！"));
            else
                qWarning() << "无法打开 JSON 文件！";
            return;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            if (browser)
                browser->append(QStringLiteral("JSON 解析失败！"));
            else
                qWarning() << "JSON 解析失败！";
            return;
        }

        QJsonObject root = doc.object();
        QJsonArray departments = root["departments"].toArray();

        bool found = false;
        for (const QJsonValue& value : departments) {
            QJsonObject dept = value.toObject();
            int x1 = dept["x1"].toInt();
            int y1 = dept["y1"].toInt();
            int x2 = dept["x2"].toInt();
            int y2 = dept["y2"].toInt();

            if (col >= x1 && col <= x2 && row >= y1 && row <= y2) {
                QString deptName = dept["name"].toString();
                if (browser)
                {
                    browser->append(QStringLiteral("📍 属于科室: %1").arg(deptName));
                }
                else
                    qDebug() << "📍 属于科室:" << deptName;
                found = true;

                break;
            }
        }

        if (!found) {
            if (browser)
                browser->append(QStringLiteral("点击位置未落入任何科室范围"));
            else
                qDebug() << "点击位置未落入任何科室范围";
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void SvgViewer::wheelEvent(QWheelEvent* event) {
    // 保留滚轮缩放功能：上滚放大，下滚缩小
    const double scaleFactor = 1.2;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}
