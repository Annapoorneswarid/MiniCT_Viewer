#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageviewer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QCollator>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->sliceSlider->setEnabled(false);

    connect(ui->btnOpenFolder,  &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(ui->sliceSlider,    &QSlider::valueChanged, this, &MainWindow::onSliceSliderChanged);
    connect(ui->btnPrevSlice,   &QPushButton::clicked, this, &MainWindow::onPrevSlice);
    connect(ui->btnNextSlice,   &QPushButton::clicked, this, &MainWindow::onNextSlice);

    connect(ui->brightnessSlider, &QSlider::valueChanged, this, &MainWindow::onBrightnessContrastChanged);
    connect(ui->contrastSlider,   &QSlider::valueChanged, this, &MainWindow::onBrightnessContrastChanged);

    connect(ui->btnZoomIn,    &QPushButton::clicked, this, &MainWindow::onZoomIn);
    connect(ui->btnZoomOut,   &QPushButton::clicked, this, &MainWindow::onZoomOut);
    connect(ui->btnFitWindow, &QPushButton::clicked, this, &MainWindow::onFitWindow);

    connect(ui->btnToolNone, &QPushButton::clicked, this, &MainWindow::onToolNone);
    connect(ui->btnToolLine, &QPushButton::clicked, this, &MainWindow::onToolLine);
    connect(ui->btnToolRect, &QPushButton::clicked, this, &MainWindow::onToolRect);
    connect(ui->btnClearAnnotations, &QPushButton::clicked, this, &MainWindow::onClearAnnotations);

    connect(ui->imageViewer, &ImageViewer::measurementChanged, this, &MainWindow::onMeasurementChanged);

    setWindowTitle("Mini CT Viewer");
    resize(1050, 780);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenFolder()
{
    const QString folder = QFileDialog::getExistingDirectory(this, "Select CT Slice Folder");
    if (!folder.isEmpty())
        loadFolder(folder);
}

void MainWindow::loadFolder(const QString &folderPath)
{
    QDir dir(folderPath);

    // Standard 2D formats are read directly via QImage.
    // .dcm files are also listed so the folder scan finds real DICOM stacks,
    // but note: plain QImage cannot decode DICOM pixel data on its own —
    // see the note in loadSlice() below for how to add real DICOM support.
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tif" << "*.tiff" << "*.dcm";
    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);

    if (files.isEmpty()) {
        QMessageBox::warning(this, "No images found",
            "No supported slice images were found in this folder.\n"
            "Supported directly: PNG, JPG, BMP, TIFF.\n"
            "DICOM (.dcm) files need a DICOM decoding library — see comments in mainwindow.cpp.");
        return;
    }

    // Natural sort so slice2.png sorts before slice10.png (plain QDir::Name would not).
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(files.begin(), files.end(), [&collator](const QString &a, const QString &b) {
        return collator.compare(a, b) < 0;
    });

    m_sliceFiles.clear();
    for (const QString &f : std::as_const(files))
        m_sliceFiles << dir.absoluteFilePath(f);

    ui->sliceSlider->setEnabled(true);
    ui->sliceSlider->setMinimum(0);
    ui->sliceSlider->setMaximum(m_sliceFiles.size() - 1);

    loadSlice(0);
    ui->imageViewer->fitToWindow();
}

void MainWindow::loadSlice(int index)
{
    if (index < 0 || index >= m_sliceFiles.size())
        return;

    // --- DICOM note -----------------------------------------------------
    // QImage can only load standard formats (PNG/JPG/BMP/TIFF/...).
    // Real .dcm files store pixel data behind a DICOM header + transfer
    // syntax, so they need a dedicated reader such as DCMTK or GDCM:
    //   DcmFileFormat file; file.loadFile(path.toStdString().c_str());
    //   DicomImage img(&file.getDataset()); ... img.getOutputData(...)
    // then wrap the raw pixel buffer in a QImage. That code slots in
    // exactly here, in place of the QImage(path) call below, once a
    // DICOM library is linked into the project.
    // ---------------------------------------------------------------------

    const QImage img(m_sliceFiles.at(index));
    if (img.isNull()) {
        QMessageBox::warning(this, "Load error",
            QString("Could not load image:\n%1").arg(m_sliceFiles.at(index)));
        return;
    }

    m_rawImage = img.convertToFormat(QImage::Format_Grayscale8);
    m_currentIndex = index;

    applyBrightnessContrast();
    updateSliceLabel();

    ui->sliceSlider->blockSignals(true);
    ui->sliceSlider->setValue(index);
    ui->sliceSlider->blockSignals(false);
}

void MainWindow::applyBrightnessContrast()
{
    if (m_rawImage.isNull())
        return;

    const int brightness = ui->brightnessSlider->value();        // -255..255
    const double contrast = ui->contrastSlider->value() / 100.0; // 0.0..3.0

    // Precompute an 8-bit lookup table once per adjustment instead of doing
    // the math per-pixel — keeps interaction responsive on large slices.
    uchar lut[256];
    for (int i = 0; i < 256; ++i) {
        double v = (i - 127.0) * contrast + 127.0 + brightness;
        v = std::clamp(v, 0.0, 255.0);
        lut[i] = static_cast<uchar>(v);
    }

    m_processedImage = m_rawImage.copy();
    for (int y = 0; y < m_processedImage.height(); ++y) {
        uchar *line = m_processedImage.scanLine(y);
        const int width = m_processedImage.width();
        for (int x = 0; x < width; ++x)
            line[x] = lut[line[x]];
    }

    ui->imageViewer->setImage(m_processedImage);
}

void MainWindow::updateSliceLabel()
{
    ui->lblSliceInfo->setText(QString("Slice %1 / %2")
                                   .arg(m_currentIndex + 1)
                                   .arg(m_sliceFiles.size()));
}

void MainWindow::onSliceSliderChanged(int value)
{
    loadSlice(value);
}

void MainWindow::onPrevSlice()
{
    if (m_currentIndex > 0)
        loadSlice(m_currentIndex - 1);
}

void MainWindow::onNextSlice()
{
    if (m_currentIndex < m_sliceFiles.size() - 1)
        loadSlice(m_currentIndex + 1);
}

void MainWindow::onBrightnessContrastChanged()
{
    applyBrightnessContrast();
}

void MainWindow::onZoomIn()  { ui->imageViewer->zoomIn(); }
void MainWindow::onZoomOut() { ui->imageViewer->zoomOut(); }
void MainWindow::onFitWindow() { ui->imageViewer->fitToWindow(); }

void MainWindow::onToolNone()
{
    setToolButtonsExclusive(ui->btnToolNone);
    ui->imageViewer->setAnnotationMode(ImageViewer::AnnotationMode::None);
}

void MainWindow::onToolLine()
{
    setToolButtonsExclusive(ui->btnToolLine);
    ui->imageViewer->setAnnotationMode(ImageViewer::AnnotationMode::Line);
}

void MainWindow::onToolRect()
{
    setToolButtonsExclusive(ui->btnToolRect);
    ui->imageViewer->setAnnotationMode(ImageViewer::AnnotationMode::Rectangle);
}

void MainWindow::onClearAnnotations()
{
    ui->imageViewer->clearAnnotations();
}

void MainWindow::onMeasurementChanged(const QString &text)
{
    ui->lblMeasurement->setText(text);
}

void MainWindow::setToolButtonsExclusive(QAbstractButton *checkedButton)
{
    const QList<QAbstractButton *> buttons = { ui->btnToolNone, ui->btnToolLine, ui->btnToolRect };
    for (auto *b : buttons)
        b->setChecked(b == checkedButton);
}
