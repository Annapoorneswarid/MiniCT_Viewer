# Mini CT Viewer (Qt + C++)

A desktop medical image viewer developed using Qt (C++) that provides interactive visualization and analysis of grayscale image slices. The application allows users to browse image sequences, adjust brightness and contrast in real time, zoom and pan images, and create simple measurement annotations.

**Features**
#Load image sequences from a selected folder.
#Natural sorting of image slices for correct viewing order.
#Interactive slice navigation using a slider and Previous/Next controls.
#Real-time brightness and contrast adjustment using a lookup table (LUT) for efficient image processing.
#Smooth zooming with mouse wheel support.
#Pan images using click-and-drag.
#Fit image to window.
#Annotation tools for drawing line and rectangle measurements.
#Live measurement display for annotation objects.
#Clear all annotations with a single action.

**Technologies Used**
#Language: C++
#Framework: Qt Widgets
#Graphics: QGraphicsView / QGraphicsScene
#Image Processing: QImage, QPixmap
#Development Environment: Qt Creator

**Project Structure**
#imageviewer.h / imageviewer.cpp – Custom image viewer widget responsible for image rendering, zooming, panning, and annotation functionality.
#mainwindow.h / mainwindow.cpp – Main application logic, image loading, slice navigation, brightness/contrast adjustment, and UI event handling.
#mainwindow.ui – User interface designed with Qt Designer.
#main.cpp – Application entry point.

**How It Works**
#Select a folder containing image slices.
#The application scans and naturally sorts all supported image files.
#Images are displayed one slice at a time in the custom viewer.
#Users can navigate through slices, adjust image brightness and contrast, zoom, pan, and add measurement annotations.
#All image interactions are handled efficiently using Qt's Graphics View Framework, providing smooth rendering even for large images.

