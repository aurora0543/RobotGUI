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
    QGraphicsScene* scene;           // show SVG
    QGraphicsSvgItem* backgroundItem; // show background

signals:
    void departmentSelected(const QString& name);
};

#endif // SVGVIEWER_H
