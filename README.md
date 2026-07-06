# Mini CT Viewer (Qt + C++)

## Files
- `main.cpp` — app entry point
- `mainwindow.h` / `mainwindow.cpp` / `mainwindow.ui` — main window: folder loading, slice navigation, brightness/contrast controls, annotation toolbar
- `imageviewer.h` / `imageviewer.cpp` — custom `QGraphicsView` widget: zoom, pan, and line/rectangle annotation with pixel measurement
- `CMakeLists.txt` — CMake build (Qt6, falls back to Qt5)
- `MiniCTViewer.pro` — qmake build, if you prefer Qt Creator's classic project file

