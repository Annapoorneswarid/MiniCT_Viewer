#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QImage>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFolder();
    void onSliceSliderChanged(int value);
    void onPrevSlice();
    void onNextSlice();
    void onBrightnessContrastChanged();
    void onZoomIn();
    void onZoomOut();
    void onFitWindow();
    void onToolNone();
    void onToolLine();
    void onToolRect();
    void onClearAnnotations();
    void onMeasurementChanged(const QString &text);

private:
    Ui::MainWindow *ui;

    QStringList m_sliceFiles;
    int m_currentIndex = -1;
    QImage m_rawImage;        // original slice converted to 8-bit grayscale
    QImage m_processedImage;  // after brightness/contrast LUT applied

    void loadFolder(const QString &folderPath);
    void loadSlice(int index);
    void applyBrightnessContrast();
    void updateSliceLabel();
    void setToolButtonsExclusive(QAbstractButton *checkedButton);
};

#endif // MAINWINDOW_H
