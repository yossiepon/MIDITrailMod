[🇯🇵 日本語](README.ja.md)

# MIDITrail Mod

MIDITrail is a MIDI player which provides 3D visualization of MIDI datasets.
MIDITrail supports SMF format 0/1, and multiple MIDI ports.

This is **yossiepon Mod** — a fork of
[MIDITrail by WADA Masashi](https://www.yknk.org/miditrail/) with additional
features: ripple/keyboard effect enhancements, auto-switching keyboard
orientation, multi-port piano keyboard display, lyrics support, and more.

## System requirement

- OS: Windows 10 / 11
- VIDEO: Support for DirectX 11 graphics

A GPU with DirectX 11 support is required.
High performance graphics hardware is recommended for smooth animation,
especially with MIDI files containing a large number of notes.

## Quick Start

1. Drop a Standard MIDI file (\*.mid) onto the window.
2. Press SPACE to play/pause, ESC to stop.
3. Use W/A/S/D keys and mouse to move the viewpoint in 3D space.

For detailed operation instructions, see "Help" > "Manual...".

## Build

### Requirements

- Visual Studio 2026 (v18) or later
- Windows SDK 10.0
- vcpkg (manifest mode — dependencies are managed via `vcpkg.json`)

### Steps

```
MSBuild MIDITrail.sln /p:Configuration=Release /p:Platform=x64
```

Or open `MIDITrail.sln` in Visual Studio and build (Release / x64).

vcpkg manifest mode automatically installs the required packages (DirectXTK)
on the first build.

## Folders

| Folder | Description |
|---|---|
| /MIDITrail | Application project (MIDITrail.exe). Implements rendering using DirectX 11. Uses SMIDILib.dll and YNBaseLib.dll. |
| /SMIDILib | Simple MIDI Library project (SMIDILib.dll). Implements MIDI playback and note information analysis. Uses YNBaseLib.dll. |
| /YNBaseLib | Basic Library project (YNBaseLib.dll). Implements error control and utility functions. |
| /Resources | Resource files referenced by the application (distributed with the binary). |
| /x64 | Build output for x64 (Debug / Release). |

## License

MIDITrail is released under the BSD 3-Clause License.
See [COPYRIGHT.TXT](COPYRIGHT.TXT) and [LICENSE.TXT](LICENSE.TXT) for details.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).
