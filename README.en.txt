******************************************************************************

  MIDITrail source code Ver.1.5.0 for Windows

  Copyright (C) 2010-2026 WADA Masashi. All Rights Reserved.

  Web : https://www.yknk.org/miditrail/
  Mail: wada@yknk.org

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

  /SoundLib/TinySoundFont
	TinySoundFont library.
	This library loads an SF2 file and renders the waveform.

  /Resources
    The resource files referenced by application.

(4) License

  MIDITrail is released under the BSD license.
  Please check "LICENSE.txt".

(5) Acknowledgements

  MIDITrail uses the following library and SoundFont bank.
  Please refer to the included license file for details on each license.

  TinySoundFont : Copyright (C) 2017-2023 Bernhard Schelling (Based on SFZero: Copyright (C) 2012 Steve Folta)
	https://github.com/schellingb/TinySoundFont
    License details : SoundLib/TinySoundFont/LICENSE

  GeneralUser GS : Copyright (C) 1997-2025 S. Christian Collins
    https://www.schristiancollins.com/
    License details : Resources/wave/LICENSE.txt


