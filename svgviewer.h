#ifndef SVGVIEWER_H
#define SVGVIEWER_H

#include <QGraphicsView>
#include <QString>

class QGraphicsScene;
class QGraphicsSvgItem;

class SvgViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SvgViewer(QWidget* parent = nullptr);
    void loadSvg(const QString &filePath);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QGraphicsScene* scene;           // 场景
    QGraphicsSvgItem* backgroundItem; // 显示SVG的背景图层

signals:
    void departmentSelected(const QString& name);  // 添加这个信号
};

#endif // SVGVIEWER_H
