# Microsoft Developer Studio Project File - Name="sdl_sound_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sdl_sound_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sdl_sound_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sdl_sound_static.mak" CFG="sdl_sound_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sdl_sound_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sdl_sound_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sdl_sound_static - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sdl_sound_static___Win32_Release"
# PROP BASE Intermediate_Dir "sdl_sound_static___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sdl_sound_static___Win32_Debug"
# PROP Intermediate_Dir "sdl_sound_static___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\\" /I "..\decoders" /I "..\decoders\timidity" /I "..\decoders\mpglib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "SOUND_SUPPORTS_AU" /D "SOUND_SUPPORTS_AIFF" /D "SOUND_SUPPORTS_SHN" /D "SOUND_SUPPORTS_MIDI" /D "SOUND_SUPPORTS_WAV" /D "SOUND_SUPPORTS_VOC" /D "SOUND_SUPPORTS_MIKMOD" /D "SOUND_SUPPORTS_MPGLIB" /D "SOUND_SUPPORTS_SMPEG" /D "SOUND_SUPPORTS_OGG" /D "SOUND_SUPPORTS_RAW" /D "SOUND_SUPPORTS_MODPLUG" /D "SOUND_SUPPORTS_FLAC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sdl_sound_static - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sdl_sound_static___Win32_Debug"
# PROP BASE Intermediate_Dir "sdl_sound_static___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sdl_sound_static___Win32_Debug"
# PROP Intermediate_Dir "sdl_sound_static___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\decoders" /I "..\decoders\timidity" /I "..\decoders\mpglib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "SOUND_SUPPORTS_AU" /D "SOUND_SUPPORTS_AIFF" /D "SOUND_SUPPORTS_SHN" /D "SOUND_SUPPORTS_MIDI" /D "SOUND_SUPPORTS_WAV" /D "SOUND_SUPPORTS_VOC" /D "SOUND_SUPPORTS_MIKMOD" /D "SOUND_SUPPORTS_MPGLIB" /D "SOUND_SUPPORTS_SMPEG" /D "SOUND_SUPPORTS_OGG" /D "SOUND_SUPPORTS_RAW" /D "SOUND_SUPPORTS_MODPLUG" /D "SOUND_SUPPORTS_FLAC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"win32lib\sdl_sound_static_d.lib"

!ENDIF 

# Begin Target

# Name "sdl_sound_static - Win32 Release"
# Name "sdl_sound_static - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "decoders"

# PROP Default_Filter ""
# Begin Group "mpglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\decoders\mpglib\dct64_i386.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\decode_i386.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\huffman.h
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\interface.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\l2tables.h
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\layer1.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\layer2.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\layer3.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\mpg123_sdlsound.h
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\mpglib_common.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\mpglib_sdlsound.h
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib\tabinit.c
# End Source File
# End Group
# Begin Group "timidity"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\decoders\timidity\common.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\common.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\instrum.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\instrum.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\mix.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\mix.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\options.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\output.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\output.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\playmidi.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\playmidi.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\readmidi.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\readmidi.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\resample.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\resample.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\tables.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\tables.h
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\timidity.c
# End Source File
# Begin Source File

SOURCE=..\decoders\timidity\timidity.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\decoders\aiff.c
# End Source File
# Begin Source File

SOURCE=..\decoders\au.c
# End Source File
# Begin Source File

SOURCE=..\decoders\flac.c
# End Source File
# Begin Source File

SOURCE=..\decoders\midi.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mikmod.c
# End Source File
# Begin Source File

SOURCE=..\decoders\modplug.c
# End Source File
# Begin Source File

SOURCE=..\decoders\mpglib.c
# End Source File
# Begin Source File

SOURCE=..\decoders\ogg.c
# End Source File
# Begin Source File

SOURCE=..\decoders\raw.c
# End Source File
# Begin Source File

SOURCE=..\decoders\shn.c
# End Source File
# Begin Source File

SOURCE=..\decoders\smpeg.c
# End Source File
# Begin Source File

SOURCE=..\decoders\voc.c
# End Source File
# Begin Source File

SOURCE=..\decoders\wav.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\acconfig.h
# End Source File
# Begin Source File

SOURCE=..\audio_convert.c
# End Source File
# Begin Source File

SOURCE=..\extra_rwops.c
# End Source File
# Begin Source File

SOURCE=..\extra_rwops.h
# End Source File
# Begin Source File

SOURCE=..\SDL_sound.c
# End Source File
# Begin Source File

SOURCE=..\SDL_sound.h
# End Source File
# Begin Source File

SOURCE=..\SDL_sound_internal.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
