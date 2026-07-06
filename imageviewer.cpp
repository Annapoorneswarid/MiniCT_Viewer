#include "imageviewer.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <cmath>

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    m_pixmapItem = new QGraphicsPixmapItem();
    m_scene->addItem(m_pixmapItem);

    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setDragMode(QGraphicsView::ScrollHandDrag);          // default = pan
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate); // avoids full redraws
    setBackgroundBrush(QBrush(Qt::black));
}

void ImageViewer::setImage(const QImage &image)
{
    if (image.isNull())
        return;

    m_pixmapItem->setPixmap(QPixmap::fromImage(image));
    m_scene->setSceneRect(m_pixmapItem->boundingRect());
}

void ImageViewer::resetView()
{
    m_zoomFactor = 1.0;
    resetTransform();
    emit zoomFactorChanged(m_zoomFactor);
}

void ImageViewer::zoomIn()
{
    if (m_zoomFactor * m_zoomStep > m_maxZoom)
        return;
    m_zoomFactor *= m_zoomStep;
    scale(m_zoomStep, m_zoomStep);
    emit zoomFactorChanged(m_zoomFactor);
}

void ImageViewer::zoomOut()
{
    if (m_zoomFactor / m_zoomStep < m_minZoom)
        return;
    m_zoomFactor /= m_zoomStep;
    scale(1.0 / m_zoomStep, 1.0 / m_zoomStep);
    emit zoomFactorChanged(m_zoomFactor);
}

void ImageViewer::fitToWindow()
{
    if (m_pixmapItem->pixmap().isNull())
        return;
    fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    m_zoomFactor = transform().m11();
    emit zoomFactorChanged(m_zoomFactor);
}

void ImageViewer::setAnnotationMode(AnnotationMode mode)
{
    m_annotationMode = mode;
    // Disable panning while an annotation tool is active so clicks draw instead of drag the view.
    setDragMode(mode == AnnotationMode::None ? QGraphicsView::ScrollHandDrag
                                              : QGraphicsView::NoDrag);
}

void ImageViewer::clearAnnotations()
{
    const auto items = m_scene->items();
    for (auto *item : items) {
        if (item != m_pixmapItem) {
            m_scene->removeItem(item);
            delete item;
        }
    }
    emit measurementChanged(QString());
}

double ImageViewer::distance(const QPointF &a, const QPointF &b)
{
    return std::sqrt(std::pow(b.x() - a.x(), 2) + std::pow(b.y() - a.y(), 2));
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0)
        zoomIn();
    else
        zoomOut();
    event->accept();
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (m_annotationMode == AnnotationMode::None || event->button() != Qt::LeftButton) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    m_drawing = true;
    m_startScenePos = mapToScene(event->pos());

    QPen pen(Qt::yellow);
    pen.setWidth(0);        // 0 = cosmetic width in Qt (always 1 device pixel)
    pen.setCosmetic(true);  // keep line thickness constant regardless of zoom

    if (m_annotationMode == AnnotationMode::Line) {
        auto *line = new QGraphicsLineItem(QLineF(m_startScenePos, m_startScenePos));
        line->setPen(pen);
        m_scene->addItem(line);
        m_currentAnnotation = line;
    } else if (m_annotationMode == AnnotationMode::Rectangle) {
        auto *rect = new QGraphicsRectItem(QRectF(m_startScenePos, QSizeF(0, 0)));
        rect->setPen(pen);
        m_scene->addItem(rect);
        m_currentAnnotation = rect;
    }
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawing || !m_currentAnnotation) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    const QPointF currentPos = mapToScene(event->pos());

    if (m_annotationMode == AnnotationMode::Line) {
        if (auto *line = qgraphicsitem_cast<QGraphicsLineItem *>(m_currentAnnotation)) {
            line->setLine(QLineF(m_startScenePos, currentPos));
            const double d = distance(m_startScenePos, currentPos);
            emit measurementChanged(QString("Line length: %1 px").arg(d, 0, 'f', 2));
        }
    } else if (m_annotationMode == AnnotationMode::Rectangle) {
        if (auto *rect = qgraphicsitem_cast<QGraphicsRectItem *>(m_currentAnnotation)) {
            const QRectF r = QRectF(m_startScenePos, currentPos).normalized();
            rect->setRect(r);
            emit measurementChanged(QString("Rect: %1 x %2 px")
                                         .arg(r.width(), 0, 'f', 2)
                                         .arg(r.height(), 0, 'f', 2));
        }
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_drawing && event->button() == Qt::LeftButton) {
        m_drawing = false;
        m_currentAnnotation = nullptr;
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}
