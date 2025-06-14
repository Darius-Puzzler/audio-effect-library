# 3rd Order Tunable Butterworth Low-Pass Filter

## Overview
A real-time tunable 3rd Order Butterworth Low-Pass Filter (LPF) implemented using Wave Digital Filter (WDF) technology. This iPlug2 plugin demonstrates frequency scaling for dynamic filter cutoff adjustment.

## Features
- **Real-time Tunable**: Adjust the cutoff frequency on the fly
- **WDF Implementation**: Uses Wave Digital Filters for accurate analog modeling
- **Frequency Scaling**: Implements component value scaling for different cutoff frequencies
- **iPlug2 Framework**: Built using the iPlug2 C++ framework for audio plugins

## Technical Details

### Filter Design
- **Type**: 3rd Order Butterworth Low-Pass Filter
- **Topology**: Ladder filter using WDF adaptors
- **Components**:
  - Series Inductor (L1)
  - Parallel Capacitor (C1)
  - Series-Teminated Inductor (L2)
- **Frequency Response**: -60dB/decade roll-off

### Implementation
- **Sampling Rate**: 44.1 kHz
- **Cutoff Range**: 60 Hz to 20.48 kHz
- **Source/Termination**: 600Ω impedance matching
- **Processing**: Sample-by-sample processing for maximum compatibility

## Building
### Prerequisites
- iPlug2 framework
- C++17 compatible compiler
- CMake (version 3.15 or later)

### Build Instructions
```bash
# Clone the repository
git clone [repository-url]
cd audio-effect-library

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DIPLUG_ROOT=path/to/iplug2
cmake --build . --config Release
```

## Plugin Controls
- **Gain**: Adjusts output level (0-100%)
- **Cutoff (fc)**: Sets the filter cutoff frequency (60Hz - 20.48kHz)

## Usage
```cpp
// Example of processing audio samples
WDFTunableButterLPF3 filter;
filter.reset(sampleRate); // Reset with current sample rate
filter.calculateNewComponentValues(1000.0f); // Set cutoff to 1kHz

double output = filter.processAudioSample(inputSample);
```

## Implementation Notes
- The filter uses frequency warping to correct for the bilinear transform's frequency distortion
- Component values are automatically scaled when the cutoff frequency changes
- The WDF structure is optimized for real-time performance

## License
[Specify your license here]