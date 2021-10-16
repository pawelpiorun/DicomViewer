# DicomViewer
DICOM images and volumes viewer with advanced processing infrastructure

## Stack:
- WPF (C#)
- ITK (C++)
- VTK (C++)

## Structure:
- BimaxViewer - WPF app with business logic of the application
- BimaxProcessing - ITK & VTK logic for processing and rendering data

The app is developed with MVVM pattern sense. PInvoke was used to integrate C++ and C# libraries.

## Usage
Originally it was meant to be a processing and analysis tool for DICOM volumes before and after BIMAX operations.
However, final solution became a bit too specific, so you will find some weird places in the code and UI.
Fortunately, the solution may be used to process and analyse any data (2D images, 3D DICOMS for now), just requires a bit of effort to extend processing algorithms. :)
