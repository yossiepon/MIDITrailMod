******************************************************************************

  MIDITrail source code Ver.1.3.6 for Windows

  Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.

  Web : https://osdn.jp/projects/miditrail/
  Mail: yknk@users.osdn.me

******************************************************************************

(1) Introduction

  This is the entire source code of MIDITrail for Windows.

(2) Development environment

  Microsoft DirectX SDK (June 2010)
  Microsoft Visual Studio Community 2017

(3) Folders

  /bin
    The output folder of the builded modules for x86.
    The "Debug" and "Release" folder are created under the "bin" folder.

  /x64
    The output folder of the builded modules for x64.
    The "Debug" and "Release" folder are created under the "x64" folder.

  /MIDITrail
    The application project. (MIDITrail.exe)
    It implements the processing of rendering using DirectX.
    It uses "SMIDILib.dll" and "YNBaseLib.dll".

  /SMIDILib
    The Simple MIDI Library project. (SMIDILib.dll)
    It implements the processing of MIDI control and analyzing note informantion.
    It uses "YNBaseLib.dll".

  /YNBaseLib
    The Basic Library Project. (YNBaseLib.dll)
    It implements the error control and utility functions.

  /Resources
    The resource files referenced by application.

(4) License

  MIDITrail is released under the BSD license.
  Please check "LICENSE.txt".


