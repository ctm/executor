README
2002/12/23

This directory contains a Project Workspace for Visual Studio 6.
It is based off the code in the CVS after the 1.0.0 release (1.0.1?).


This project has options to build a DLL or static library and 
build playsound (dynamic or static). This project mimics the 
original pre-1.0.0 version which no longer works with the current 
code.

Unlike the former package, this one contains no binaries. This
will allow this project to be included in the main SDL_sound
source code. You will be responsible for finding the binaries
you need for each decoder. We have attempted to provide a 
Support pack which contains the binaries built and tested with. 
However, many of the binaries become quickly outdated so 
you may not want to depend too heavily on the Support pack.

If you need the binaries, you should either copy the files to your 
default VisualC++ directories (both headers and libraries), or add 
them to your search paths, either through the Project Settings for 
Include and Link paths or through the global settings 
(Tools->Options->Directories in VC6). You need to do it both for the 
header files (includes) and library files. To run your final 
executables, you will need the DLL files in the local path
or one of your Windows main DLL search paths.


Issues: 

The static playsound really isn't static. You still need the 
dlls for each of the codecs. You will have to tweak the project to 
build a true static binary and will probably require you to have 
static versions of all the decoder libraries.




Eric Wing <ewing2121@yahoo.com>
Joshua Quick <jquick@golighthouse.com>
