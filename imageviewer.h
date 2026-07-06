#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPointF>

// ImageViewer renders one CT slice at a time using QGraphicsView/QGraphicsScene.
// QGraphicsView already caches and only redraws what changed, so zoom/pan stay
// smooth even on large images. It also supports mouse-driven zoom, click+drag
// pan, and two simple annotation tools (line / rectangle) with live pixel
// measurements.
class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    enum class AnnotationMode { None, Line, Rectangle };

    explicit ImageViewer(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    void resetView();
    void zoomIn();
    void zoomOut();
    void fitToWindow();

    void setAnnotationMode(AnnotationMode mode);
    void clearAnnotations();

signals:
    void measurementChanged(const QString &text);
    void zoomFactorChanged(double factor);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;

    AnnotationMode m_annotationMode = AnnotationMode::None;
    bool m_drawing = false;
    QPointF m_startScenePos;
    QGraphicsItem *m_currentAnnotation = nullptr;

    double m_zoomFactor = 1.0;
    static constexpr double m_zoomStep = 1.15;
    static constexpr double m_minZoom = 0.1;
    static constexpr double m_maxZoom = 20.0;

    static double distance(const QPointF &a, const QPointF &b);
};

#endif // IMAGEVIEWER_H
