#include "svgviewer.h"
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <QFile>
#include <QDebug>

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
    // 将点击位置转换成 scene 坐标（视图坐标与 scene 坐标不再因拖拽而偏移）
    QPointF scenePos = mapToScene(event->pos());
    qDebug() << "点击的 scene 坐标:" << scenePos;

    // 计算 scene 坐标在背景区域内的相对位置（0～1）
    QRectF rect = scene->sceneRect();
    if (!rect.isValid() || rect.isEmpty()) {
        qDebug() << "❌ 无法获取背景区域信息！";
    } else {
        double relativeX = (scenePos.x() - rect.x()) / rect.width();
        double relativeY = (scenePos.y() - rect.y()) / rect.height();
        // 映射到 100×50 的矩阵中
        int col = static_cast<int>(relativeX * 100);
        int row = static_cast<int>(relativeY * 50);
        qDebug() << "映射到矩阵 (100x50) 中的坐标:" << "(" << col << "," << row << ")";
    }

    // 调用基类以保持其他鼠标事件行为
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
