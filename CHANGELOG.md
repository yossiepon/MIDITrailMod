# Changelog

All notable changes to MIDITrail Mod are documented in this file.

## References

- osdn#\<id\>: [OSDN Ticket Archive](https://www.yknk.org/miditrail/osdn/ticket/index.html) (individual tickets: `https://www.yknk.org/miditrail/osdn/ticket/<id>.html`)
- ghwin#\<id\>: [GitHub MIDITrail-Windows Issues](https://github.com/wdmss/MIDITrail-Windows/issues)

---

## DX11 Migration (2026-08)

Ported the rendering engine from Direct3D 9 to Direct3D 11 using DirectXTK SimpleMath.
Based on yossiepon Mod (not ced's ModMod), but GPU instanced rendering for
MTNoteBoxMod11, MTNoteRippleMod11, and MTNoteLyricsMod11 references
[ced's MIDITrail Mod Mod](https://github.com/Zel9278/MIDITrailModMod).

- Replaced DX9 fixed-function pipeline with DX11 shader-based rendering
- Replaced DirectX SDK (June 2010) / d3dx9 dependency with Windows SDK + DirectXTK
- Implemented GPU instanced rendering for note boxes, ripples, and lyrics
- Implemented Live monitoring for all scenes (3D/2D/Rain/Rain2D/Ring)
- Separated Playback and Live into distinct scene classes with shared base
- Changed development environment to Visual Studio 2026 / PlatformToolset v145

---

## yossiepon Mod

### Mod 20251101 (based on MIDITrail Ver.1.4.1)

- [FIX] PianoRollRing: Fixed issues in Ripple Mod (since Mod 20191224, unreleased)
- [ADD] PianoRollRing: Support for Lyrics display
- [FIX] Lyrics Display: Resolved an assertion error that occurred when running in debug mode
- [FIX] Add File Menu: Fixed a bug where channel number override was not processed

### Mod 20251030 (based on MIDITrail Ver.1.4.1)

- [MERGE] Merged original Ver.1.4.0 - Ver.1.4.1

### Mod 20250616 (based on MIDITrail Ver.1.3.6)

- [MERGE] Merged original Ver.1.3.1 - Ver.1.3.6

### Mod 20191224

- [UPDATE] PianoRollRing: The ripple is modded

### Mod 20190828 (based on MIDITrail Ver.1.2.6)

- [MERGE] Merged original Ver.1.2.4 - Ver.1.2.6
- [FIX] Disable FPU precision change on x64 binary (unsupported on x64)

### Mod 20180412 (based on MIDITrail Ver.1.2.3)

- [MERGE] Merged original Ver.1.2.3
- [ADD] Ripple blend mode setting (INI: Ripple section, SrcBlend / DestBlend)
- [FIX] PianoRoll2D: Multi-port keyboard not aggregated into single keyboard
- [FIX] PianoRoll3D: Multi-port keyboard base position offset

### Mod 20170528

- [FIX] Display filename as fallback when sequence title is empty
- [FIX] Apply RTRIM when retrieving text from sequence

### Mod 20161226 (based on MIDITrail Ver.1.2.2)

- [MERGE] Integrated deferred features from original Ver.1.2.1: osdn#30547 scale color assignment, osdn#32427 note color assignment to active key
- [UPDATE] PianoRoll2D/3DMod: ActiveKeyColorType=NOTE support
- [ADD] Ripple overwrite count and spacing settings (INI: Ripple section, OverwriteTimes / Spacing)

### Mod 20161223 (based on MIDITrail Ver.1.2.2)

- [MERGE] Rebased on original Ver.1.2.2

### Mod 20140920 (based on MIDITrail Ver.1.2.1b)

- [MERGE] Rebased on original Ver.1.2.1b
- [ADD] x64 binary support

### Mod 20121229

- [ADD] PianoRoll3D: Added reverse-direction light

### Mod 20120728-30

- Refactored mod code into separate source files
- [FIX] Keyboard rendering issue when viewed from the direction of travel
- [FIX] PianoRollRain not working
- [NEW] Auto-orient keyboard direction based on piano roll pitch direction
- [NEW] PianoRoll3D: Per-port keyboard display for multi-port MIDI
- [NEW] Lyrics meta event display
- [NEW] File add-loading (File > Add menu) with port/channel auto-detection from filename

### Mod 20120318-23

- [UPDATE] Reversed piano roll pitch direction, keyboard faces direction of travel
- [UPDATE] Semi-transparent keyboard allows other objects to show through
- [UPDATE] Ripple glow effect (BLEND_ONE x3 overwrite)
- [UPDATE] Ripple/active note decay uses sequencer time instead of real time
- [UPDATE] Ripple/active note decay uses exponential curve instead of linear
- [UPDATE] Ripple/active note decay duration considers note length
- [UPDATE] Animated 2D/3D keyboard
- [UPDATE] Time display in milliseconds
- [FIX] Window background brush fixed to black (prevents occasional white flash)

---

## MIDITrail Original (by WADA Masashi)

### Ver.1.4.1 (2025-10-30)

- ghwin#7 Fixed bug (Piano roll is black on AMD Radeon Graphics)
- ghwin#8 Added function to send note off when paused or stopped and note on when resumed
- ghwin#9 Changed documents due to the migration of the project website

### Ver.1.4.0 (2022-10-20)

- osdn#45896 Added feature of color configuration
- osdn#45897 Added bar ring for Piano Roll Ring
- osdn#45898 Added Unicode support for reading MIDI files
- osdn#45899 Changed key color of Piano Roll Rain
- osdn#45900 Fixed bug (Counter displays monitoring stop when toggle full screen)
- osdn#45901 Fixed bug (Window size is smaller than user setting)
- osdn#45931 Fixed bug (MIDITrail is not reading tempo data correctly)

### Ver.1.3.6 (2022-07-14)

- osdn#45088 Added menu bar display switch
- osdn#45089 Added feature of saving my viewpoint
- osdn#45090 Added display switch of grid line and time indicator
- osdn#45091 Added feature of saving display switch status
- osdn#45092 Added 2nd light for Piano Roll 3D
- osdn#45093 Added MIDI system message parsing

### Ver.1.3.4 (2021-09-13)

- osdn#42859 Added setting of delay between songs

### Ver.1.3.3 (2021-05-16)

- osdn#42233 Added Folder Playback feature
- osdn#42234 Added support for Standard MIDI File with illegal chunk size
- osdn#42235 Added support for RIFF-based MIDI File
- osdn#42236 Added sending "All Sound Off" message when pause/stop/skip
- osdn#42237 Fixed bug (The color of the ripples becomes dark in Piano Roll 3D)

### Ver.1.3.1 (2019-11-09)

- osdn#39733 Changed keyboard display direction on Piano Roll Ring
- osdn#39734 Changed processing to keep viewpoint when window size changed
- osdn#39735 Fixed bug (Assertion occurs in _controlfp_s when x64 debug configuration)
- osdn#39736 Fixed bug (Exception occurs in "How to view" dialog when x64 debug configuration)

### Ver.1.3.0 (2019-10-14)

- osdn#39675 Added view mode "Piano Roll Ring"

### Ver.1.2.6 (2019-06-02)

- osdn#39274 Added full screen mode
- osdn#39275 Added support for game controller

### Ver.1.2.5 (2019-05-12)

- osdn#39227 Added quarter note length configuration
- osdn#39226 Changed development environment to Visual Studio 2017
- osdn#39228 Changed appendix of user's manual
- osdn#39229 Fixed bug (Code analysis found some mistakes)

### Ver.1.2.4 (2019-02-10)

- osdn#38953 Added viewpoint selection menu
- osdn#38954 Fixed bug (Piano roll bar flickers in Piano Roll 2D mode)
- osdn#38955 Fixed bug (Time indicator becomes opaque in Piano Roll 3D mode)

### Ver.1.2.3 (2017-10-09)

- osdn#37561 Added increase of the size of active note
- osdn#37562 Changed brightness the ripple color

### Ver.1.2.2 (2016-12-05)

- osdn#35775 Changed URL and mail address in user's manual
- osdn#36844 Added supports window size customization
- osdn#36845 Added supports background image customization
- osdn#36846 Changed version information string

### Ver.1.2.1b (2014-04-20)

- osdn#33674 Fixed bug (When NSX-39 was selected as MIDI OUT device, error occurs at the start of playback)
- osdn#33695 Fixed bug (Program error occurs in MTFont2Bmp::_writeGlyphToBmpBuf)

### Ver.1.2.1a (2014-02-23)

- osdn#33252 Added support for Windows 64bit

### Ver.1.2.1 (2013-12-04)

- osdn#30545 Added key display range control
- osdn#30546 Added back ground color control
- osdn#30547 Added scale color assignment to the piano roll bar
- osdn#30553 Added ability to render the user-specified 3D model
- osdn#32360 Added menu item "Auto save viewpoint"
- osdn#32361 Added ability to save viewpoint in MIDI IN monitoring
- osdn#32362 Added ability to playback automatically after opening file
- osdn#32363 Added ability to open the file while playing
- osdn#32364 Added ability to inhibit multiple instances of MIDITrail
- osdn#32366 Added ability to display the file name
- osdn#32367 Added view mode "Piano Roll Rain 2D"
- osdn#32427 Added note color assignment to active key
- osdn#32365 Changed title caption ("TITLE:" string was removed)
- osdn#30552 Improved event message queue mechanism
- osdn#30548 Fixed bug (At the start of playback, MIDI data transmission is blocked)
- osdn#32359 Fixed bug (MIDITrail does not send some CC events at skip processing)

### Ver.1.2.0 (2012-03-04)

- osdn#27711 Merged features of MIDITrail Ver.1.2.0 for Mac OS X
- osdn#27458 Added MIDI IN monitoring
- osdn#27459 Added MIDI OUT auto configuration
- osdn#27712 Added shortcut key of File Open
- osdn#27725 Fixed bug (Error occurs when the MIDI data contains the track with 6 or more port numbers)

### Ver.1.1.3 (2012-01-15)

- osdn#27006 Merged features of MIDITrail Ver.1.1.3 for Mac OS X
- osdn#25454 Added grid line display control
- osdn#26987 Added playback speed control
- osdn#26988 Added skip control
- osdn#26989 Added stars and counter display control
- osdn#27005 Added figure of player operations guide to "How to view dialog"
- osdn#27091 Added check of DirectX feature support (Index Buffer)
- osdn#26990 Fixed bug (Typographical errors on the license)
- osdn#27139 Fixed bug (The color of note boxes are whitish in Piano Roll 3D)
- osdn#27140 Fixed bug (The color of stars are dark in Piano Roll Rain)

### Ver.1.1.2 (2010-12-26)

- This version was released with MIDITrail for Mac OS X
- osdn#23970 Added eye direction control by key operation
- osdn#23971 Updated texture images
- osdn#23972 Updated application icon
- osdn#23497 Fixed bug ("Piano Roll Rain" item in "View" menu is always enable)
- osdn#23968 Fixed bug (Time signature was always 4/4 after loading file)
- osdn#23969 Fixed bug (Space character in the title string has been ignored)

### Ver.1.1.1 (2010-10-17)

- osdn#23439 Added Anti-aliasing support
- osdn#23347 Fixed bug (Key down timing and a piano roll don't synchronize in PianoRollRain)
- osdn#23348 Fixed bug (Spelling error in file open dialog)
- osdn#23422 Fixed bug (Position of black is not correct in PianoRollRain)

### Ver.1.1.0 (2010-09-19)

- osdn#23174 Added view mode "Piano Roll Rain"
- osdn#23175 Fixed bug (File open dialog is not modal)

### Ver.1.0.5 (2010-08-06)

- osdn#22633 Fixed bug (Piano roll rotation status is not saved by "Save viewpoint")
- osdn#22652 Fixed bug (Parse error when opening 55MCMDL1.MID "WATANABE Michiaki MEDLEY")
- osdn#22709 Fixed bug (MIDITrail sends redundant "F7" to MIDI-OUT with multi-packet system exclusive message)
- osdn#22710 Fixed bug (Auto rotation speed of piano roll is not constant)

### Ver.1.0.4 (2010-07-25)

- osdn#22320 Added application icon
- osdn#22522 Added support for Recomposer data file
- osdn#22523 Added command line interface
- osdn#22582 Added double speed playback mode
- osdn#22585 Added function of piano roll rotation by a mouse wheel
- osdn#22566 Fixed bug (Playback time was slightly short)
- osdn#22506 Fixed bug (Note may continue sounding after the end of playback)

### Ver.1.0.3 (2010-07-11)

- osdn#22319 Added color effect to active piano roll bar
- osdn#22412 Added pitch bend effect to piano roll bar
- osdn#22425 Added view mode of piano roll 2D
- osdn#22442 Added switch interface of display and effect
- osdn#22413 Fixed bug (rendering performance has declined by specular effect)
- osdn#22414 Fixed bug (total count of notes is incorrect)
- osdn#22424 Fixed bug (duration of a ripple is not constant)

### Ver.1.0.2 (2010-06-20)

- osdn#22218 Added English manual
- osdn#22108 Fixed bug (crash when opening the MIDI file exported from VOCALOID)
- osdn#22217 Fixed bug (overlapped ripples flicker)

### Ver.1.0.1 (2010-06-05)

- osdn#22102 Fixed bug (execution fails by missing Visual C++ runtime library)
- osdn#22106 Fixed bug (size of about dialog is big on Windows English version)

### Ver.1.0.0 (2010-06-02)

- First formal release

### Ver.1.0.0 beta (2010-05-18)

- Beta release
