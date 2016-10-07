Bakery - Short user guide

Philipp Naumann and Marcus Soll

------------------------------------------------------------------------

Contents:

    0. Introduction
    1. Dependencies
    2. Building
    3. Graphical user interface
    4. Command-line interface
    5. Source code documentation

------------------------------------------------------------------------

0. Introduction

Bakery is a framework to compute solutions for the shape packing problem
as described by Bennell and Oliveira (J. A. Bennell and J. F. Oliveira. 
A tutorial in irregular shape packing problems. The Journal of the 
Operational Research Society, 60:s93–s105, 2009).

Bakery was the winning entry of the InformatiCup 2016. For more 
information see: 
http://informaticup.gi.de/startseite/rueckblickearchiv/informaticup-2016.html

------------------------------------------------------------------------

1. Dependencies

To build and run Bakery Qt 5.4 or later is required. See 
https://www.qt.io/ for more information.

------------------------------------------------------------------------

2. Building

The recommended way to build Bakery is to use QtCreator. Open
"bakery.pro". If not already done configure the project by selecting
a kit and press the button "Configure Project". Use the "Build All"
command to start the building process.

------------------------------------------------------------------------

3. Graphical user interface

You can start the GUI from QtCreator by chosing run target "gui" (click
on the computer in the left panel) and executing the "Run" command.

------------------------------------------------------------------------

4. Command-line interface

There is no convenient way to use the command-line interface from
QtCreator. To be able to start the binary from the command
line libbakery may need to to be added to the linker's library path
on Mac, Linux:

Mac:
    export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:[path to libbakery]"
    
Linux:
    export LD_LIBRARY_PATH="$LB_LIBRARY_PATH:[path to libbakery]"
    
On Windows Qt has to be in the PATH:
Windows:
    set PATH=%PATH%;C:\Qt\Qt5.5.1\5.5\mingw492_32\bin
    (if the installed version is 5.5.1 (MinGW32))
    
By default there is no time limit. To impose a time limit use the
-t/--time-limit parameter. A full list of available parameters may
be obtained by using the switch -h/--help.

------------------------------------------------------------------------

5. Source code documentation

The source code documentation can be build by using Doxygen
(doxygen.org). By default doxygen creates output as html, latex and 
Qt helpfile.

To generate the documentation switch the working directory to the bakery
directory and use the following command:
   doxygen
