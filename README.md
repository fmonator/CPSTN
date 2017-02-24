# Detection of objects in soccer

<del>Lukas Sekerak's school project in Computer vision</del>
Fourth Year Software Engineering Capstone Project: Soccer offside detection based on Computer Vision.

### Project idea

<del>Try detect objects (players, soccer ball, referees, goal keeper) in soccer match.</del>
<del>Detect their position, movement and show picked object in ROI area.</del>
<del>More inforamtions in a [documentation](doc/documentation.pdf) or [presentation](doc/presentation.pptx).</del>
Using Lukas Sekerak's project as a base, we optimized player detection and detected field lines to find a matrix to transform perspective in order to compare positions in an attempt to find offsides.

### Requirements

- OpenCV 2.4
- log4cpp
- POCO

### Dataset videos
[Operation Agreement CNR-FIGC] (http://www.issia.cnr.it/wp/?portfolio=operation-agreement-cnr-figc-2)

T. D’Orazio, M.Leo, N. Mosca, P.Spagnolo, P.L.Mazzeo
A Semi-Automatic System for Ground Truth Generation of Soccer Video Sequences
in the Proceeding of the 6th IEEE International Conference on Advanced Video and Signal Surveillance, Genoa, Italy September 2-4 2009

## Setting Up the Environment:
### Downloads
- Visual Studio 2012 update 5
- Visual Studio Premium 2012 (use the web installer from we.onthehub)
- log4cpp 1.1.1 -> https://sourceforge.net/projects/log4cpp/files/log4cpp-1.1.x%20%28new%29/log4cpp-1.1/
- POCO 1.7.8 (basic, no dependencies) -> 

### Your machine 
- If you haven't already, look at this: http://docs.opencv.org/2.4/doc/tutorials/introduction/windows_install/windows_install.html#windowssetpathandenviromentvariable
- System properties->advanced->environment variables
- Under system variables edit Path and add this: %OPENCV_DIR%\bin
- OPENCV_DIR should already be there if you followed the instructions for openCV above (setx -m ...)

### log4cpp
- Use 7zip to extract then take everything and add them to the project (should be folders like bcb5, config, doc, ... within a parent called log4cpp)
- You have to build one specific project: log4cpp -> msvc10 -> log4cppLIB

### POCO
- Extract it, find 'components' file and add these each on a new line: Foundation/include/Poco/*,XML/include/Poco/*,JSON/include/Poco/*,Util/include/Poco/*,Net/include/Poco/*
- Command line in the folder where components is (hereafter referred to as 'root') and run 'build_vs110.cmd build all'
- Wait ten years
- Move root into your project
- Take PocoFoundationd.dll, PocoJSONd.dll, and PocoNetd.dll out of poco-1.7.8/bin and put them into project root (w/ README)

### VS Project Properties 
- configuration properties->linker->input->additional dependencies: opencv_calib3d2413d.lib;opencv_contrib2413d.lib;opencv_core2413d.lib;opencv_features2d2413d.lib;opencv_flann2413d.lib;opencv_gpu2413d.lib;opencv_highgui2413d.lib;opencv_imgproc2413d.lib;opencv_legacy2413d.lib;opencv_ml2413d.lib;opencv_nonfree2413d.lib;opencv_objdetect2413d.lib;opencv_ocl2413d.lib;opencv_photo2413d.lib;opencv_stitching2413d.lib;opencv_superres2413d.lib;opencv_ts2413d.lib;opencv_video2413d.lib;opencv_videostab2413d.lib;PocoNetd.lib;PocoFoundationd.lib;PocoJSONd.lib;%(AdditionalDependencies)
- configuration properties->linker->additional library directories: $(SOLUTIONDIR)\poco-1.7.8\lib;$(OPENCV_DIR)\lib;$(SolutionDir)\log4cpp\msvc10\log4cppLIB\Debug
- configuration properties->C/C++->general->additional include dependencies: $(SOLUTIONDIR)\poco-1.7.8\JSON\include;$(SOLUTIONDIR)\poco-1.7.8\Foundation\include;$(SOLUTIONDIR)\poco-1.7.8\Net\include;$(OPENCV_DIR)\..\..\include;$(SOLUTIONDIR)\log4cpp\include

### License

This software is released under the [MIT License](LICENSE.md).

### Credits
- <del>Ing. Wanda Benešová, PhD. - Supervisor</del>

