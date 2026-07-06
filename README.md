# Mini CT Viewer (Qt + C++)

## Files
- `main.cpp` — app entry point
- `mainwindow.h` / `mainwindow.cpp` / `mainwindow.ui` — main window: folder loading, slice navigation, brightness/contrast controls, annotation toolbar
- `imageviewer.h` / `imageviewer.cpp` — custom `QGraphicsView` widget: zoom, pan, and line/rectangle annotation with pixel measurement
- `CMakeLists.txt` — CMake build (Qt6, falls back to Qt5)
- `MiniCTViewer.pro` — qmake build, if you prefer Qt Creator's classic project file

## Build
**CMake:**
```
mkdir build && cd build
cmake ..
cmake --build .
```

**qmake / Qt Creator:** just open `MiniCTViewer.pro` and hit Run.

## How it maps to the requirements
- **Image loading**: "Open Folder" scans a directory for PNG/JPG/BMP/TIFF files, natural-sorts them (`slice2` before `slice10`), and treats them as an ordered CT stack.
- **Viewer**: `ImageViewer` (QGraphicsView + QGraphicsScene) gives GPU-friendly zoom (mouse wheel) and pan (click+drag), with smart viewport updates so redraws stay smooth.
- **Slice navigation**: slider + Prev/Next buttons; label shows "Slice X / N".
- **Windowing/contrast**: brightness (-255..255) and contrast (0-300%) sliders build an 8-bit lookup table applied to the grayscale image in real time.
- **Annotation**: toggle buttons switch the viewer into Line or Rectangle drawing mode; dragging shows a live pixel-distance / width×height readout in the status label.

## About DICOM
Qt's own `QImage` can only decode standard image formats (PNG/JPG/BMP/TIFF/...), not real DICOM (`.dcm`) pixel data, which sits behind a DICOM header and transfer-syntax encoding. The folder scan already looks for `.dcm` files, and `mainwindow.cpp` has a comment marking exactly where to plug in a real decoder — the usual choice is **DCMTK** or **GDCM**:

```cpp
DcmFileFormat file;
file.loadFile(path.toStdString().c_str());
DicomImage dicomImg(&file.getDataset());
// pull pixel buffer out and wrap it in a QImage here
```

That's a separate library dependency (with its own install/build steps), so it's kept out of this base project to keep things buildable out of the box. If your slices are already PNG/JPG (e.g. exported from a DICOM viewer) or you convert your `.dcm` stack to PNG first, everything above works unmodified.
