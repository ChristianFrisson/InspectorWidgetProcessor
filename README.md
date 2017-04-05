# InspectorWidget Processor

## Introduction

InspectorWidget is an opensource suite to track and analyze users behaviors in their applications. 

The key contributions of InspectorWidget are:
1) it works on closed applications that do not provide source code nor scripting capabilities; 
2) it covers the whole pipeline of software analysis from logging input events to visual statistics through browsing and programmable annotation; 
3) it allows post-recording logging; and 4) it does not require programming skills. To achieve this, InspectorWidget combines low-level event logging (e.g. mouse and keyboard events) and high-level screen features (e.g. interface widgets) captured though computer vision techniques. 

InspectorWidget is targeted at end users, usability experts, user experience and HCI researchers.

## Distribution

[InspectorWidget](https://github.com/InspectorWidget/InspectorWidget) is composed of three tools:
- [Collector](https://github.com/InspectorWidget/InspectorWidgetCollector): Record (screen), Log (input events + accessibility) 
- [Iterator](https://github.com/InspectorWidget/InspectorWidgetIterator): Browse (screen + input events), Program (annotations), Analyze (worflows)
- [Processor](https://github.com/InspectorWidget/InspectorWidgetProcessor): Automate (annotations)

### InspectorWidget Processor

The Processor tool is a cross-platform server for automating annotations. 

## Dependencies

InspectorWidget Processor is based on:
- [cmake-js](https://github.com/cmake-js/cmake-js) / [cmake-js](https://github.com/cmake-js/cmake-js): a Node.js/io.js native addon build tool
- [ColinH](https://github.com/ColinH) / [PEGTL](https://github.com/ColinH/PEGTL): a grammar/syntax lexer/parser
- [kwhat](https://github.com/kwhat) / [libuiohook](https://github.com/kwhat/libuiohook): a library for hooking input events
- [itseez](https://github.com/itseez) / [opencv](https://github.com/itseez/opencv): a library for computer vision
- [tesseract-ocr](https://github.com/tesseract-ocr) / [tesseract](https://github.com/tesseract-ocr/tesseract): a library for optical character recognition (OCR)
- [laysakura](https://github.com/laysakura) / [PartialCsvParser](https://github.com/laysakura/PartialCsvParser): a library for comma-separated value (CSV) file I/O
- [miloyip](https://github.com/miloyip) / [rapidjson](https://github.com/miloyip/rapidjson): a library for JSON file I/O

### macOS 10.9+ with Homebrew
Core dependencies:
```brew install opencv --with-ffmpeg --with-tbb```
```brew install tesseract --with-opencl```
```brew install cmake npm```

To build the documentation:
```brew install doxygen gnuplot graphviz wget```

### macOS 10.9+ with MacPorts
Core dependencies:
```sudo port install opencv +ffmpeg```
```sudo port install tesseract```
```sudo port install cmake npm```

To build the documentation:
```sudo port install doxygen gnuplot graphviz wget```

### Ubuntu 16.04
Core dependencies:
```
sudo apt install cmake npm libopencv-dev libtesseract-dev libxtst-dev libxt-dev libxkbfile-dev libx11-xcb-dev libxkbcommon-dev libxkbcommon-x11-dev
sudo update-alternatives --install /usr/bin/node node /usr/bin/nodejs 10
```

To build the documentation:
```sudo apt install doxygen gnuplot graphviz wget```

## Installation

First clone the repository.
Then open a terminal in the source directory (`<source_path>`), Internet connexion still required:
* update all submodules: 
```git submodule update --init```
* install node dependencies
```sudo npm i cmake-js -g```
```npm install```
* build
```cmake <source_path>```
```make```
* build the documentation
```cmake <source_path> -DBUILD_DOCUMENTATION=ON```
```make doc```

## License

InspectorWidget Processor is released under the terms of the [GPLv3](http://www.gnu.org/licenses/gpl-3.0.html) license.

## Authors
 * [Christian Frisson](http://christian.frisson.re) (initially University of Mons, now Inria Lille): creator and main developer
 * [Gilles Bailly](http://www.gillesbailly.fr) (LTCI, CNRS, Télécom-ParisTech): contributor
 * [Sylvain Malacria](http://www.malacria.fr) (INRIA Lille): contributor