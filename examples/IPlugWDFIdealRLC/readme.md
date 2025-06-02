# WDF Ideal RLC Filters for iPlug2

A simple iPlug2 plugin demonstrating Wave Digital Filter (WDF) based RLC filters.

## Requirements
- Windows 10/11
- Visual Studio 2022 (Community Edition)
- VST3 compatible DAW

## Installation
1. Clone iPlug2:
   git clone https://github.com/iPlug2/iPlug2.git

2. Copy these files to iPlug2\Examples\IPlugWDFIdealRLC\:
   - IPlugWDFIdealRLC.h
   - IPlugWDFIdealRLC.cpp
   - projects/ folder with WDF implementations

3. Generate project files:
   cd iPlug2
   python iplug2\Scripts\generate_vs2022_examples.py

4. Open in Visual Studio 2022:
   - Navigate to iPlug2\Examples\IPlugWDFIdealRLC\projects\windows-vs2022\
   - Open IPlugWDFIdealRLC.sln

5. Build the solution (F7)

## Plugin Location
- Debug: iPlug2\Examples\IPlugWDFIdealRLC\build-win\vst3\x64\Debug\IPlugWDFIdealRLC.vst3
- Release: iPlug2\Examples\IPlugWDFIdealRLC\build-win\vst3\x64\Release\IPlugWDFIdealRLC.vst3

## Controls
- Filter Type: LPF, HPF, BPF, BSF
- Fc: Cutoff frequency (20Hz - 20.48kHz)
- Q: Resonance (0.707 - 20.0)
- Gain: Output level (0-100%)

## Troubleshooting
- No sound? Check gain and filter settings
- Plugin not showing? Rescan in your DAW
- Build errors? Ensure all requirements are installed

## Based On
- Will Pirkle's "Designing Audio Effect Plugins in C++"
- iPlug2 framework

## License