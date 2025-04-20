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
    // Disable drag mode to keep the view fixed
    setDragMode(QGraphicsView::NoDrag);
}

void SvgViewer::loadSvg(const QString &filePath) {
    // Clear the scene
    scene->clear();
    backgroundItem = nullptr;

    // Load the entire SVG image using QGraphicsSvgItem
    backgroundItem = new QGraphicsSvgItem(filePath);
    if (!backgroundItem->renderer()->isValid()) {
        qWarning() << "❌ Failed to load SVG:" << filePath;
        delete backgroundItem;
        backgroundItem = nullptr;
        return;
    }
    // Keep the image visible
    backgroundItem->setOpacity(1.0);
    backgroundItem->setZValue(-1);  // Put it at the bottom
    scene->addItem(backgroundItem);

    // Set the scene area to the bounding rect of the SVG background
    QRectF rect = backgroundItem->boundingRect();
    scene->setSceneRect(rect);

    // Reset the transformation, fit the SVG into the view, and apply initial zoom (adjustable)
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
            browser->append(QStringLiteral("❌ Unable to retrieve background area information!"));
        else
            qDebug() << "❌ Unable to retrieve background area information!";
    } else {
        double relativeX = (scenePos.x() - rect.x()) / rect.width();
        double relativeY = (scenePos.y() - rect.y()) / rect.height();
        int col = static_cast<int>(relativeX * 100);
        int row = static_cast<int>(relativeY * 50);
        {
            QString msg = QStringLiteral("Mapped to matrix (100x50) coordinates: (%1,%2)").arg(col).arg(row);
            if (browser)
                browser->append(msg);
            else
                qDebug() << msg;
        }

        // Load the JSON file
        QFile file("/home/team24/RoboHospitalGuide/RobotGUI/resources/map.json");
        if (!file.open(QIODevice::ReadOnly)) {
            if (browser)
                browser->append(QStringLiteral("Failed to open JSON file!"));
            else
                qWarning() << "Failed to open JSON file!";
            return;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            if (browser)
                browser->append(QStringLiteral("Failed to parse JSON!"));
            else
                qWarning() << "Failed to parse JSON!";
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
                    browser->append(QStringLiteral("📍 Department: %1").arg(deptName));
                    emit departmentSelected(deptName);
                }
                else
                    qDebug() << "📍 Department:" << deptName;
                
                found = true;
                break;
            }
        }

        if (!found) {
            if (browser)
                browser->append(QStringLiteral("The clicked position does not fall into any department area"));
            else
                qDebug() << "The clicked position does not fall into any department area";
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void SvgViewer::wheelEvent(QWheelEvent* event) {
    // Keep scroll-to-zoom enabled: scroll up to zoom in, scroll down to zoom out
    const double scaleFactor = 1.2;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}