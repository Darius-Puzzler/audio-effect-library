# Audio Filter Library

## Description
A C++ audio filter library based on concepts from Will Pirkle's "Designing Audio Effect Plugins in C++" book. This library is a reorganization of the original code from the book, split into separate files for better readability and maintainability.

## Organization
- **Original Source**: Code from Will Pirkle's book
- **Modification**: Split into separate files for better organization
- **Homework**: Marked sections indicate exercises from the book
- **Caveat**: Homework sections may contain errors

## Features
- Modular filter design
- Support for multiple filter types (LPF, HPF, BPF, etc.)
- Configurable filter parameters
- Optimized processing algorithms
- Clean interface design

## Structure
```
audio-effect-library/
├── examples/                           # Example implementations
│   └── IPlugWDFIdealRLC/            # Example plugin with WDF RLC filters
│       ├── IPlugWDFIdealRLC.h/cpp    # Main plugin files
│       ├── WDFIdealRLCBPF.h          # Band Pass Filter
│       ├── WDFIdealRLCBSF.h          # Band Stop Filter
│       ├── WDFIdealRLCHPF.h          # High Pass Filter
│       └── WDFIdealRLCLPF.h          # Low Pass Filter
└── fxobjects/                        # Core library
    ├── include/                      # Header files
    │   ├── AudioFilter.h             # Main filter class
    │   ├── Biquad.h                  # Biquad implementation
    │   ├── Constants.h               # Mathematical constants
    │   ├── EnumsAndStructs.h         # Constants and parameter structures
    │   ├── IAudioSignalProcessor.h   # Base interface
    │   ├── IComponentAdaptor.h       # Interface for WDF components
    │   ├── WdfAdaptorBase.h          # Base class for WDF adaptors
    │   ├── WdfCapacitor.h            # Capacitor component
    │   ├── WdfEnumsStructs.h         # WDF-specific enums and structs
    │   ├── WdfInductor.h             # Inductor component
    │   ├── WdfParallel*.h            # Parallel circuit components
    │   ├── WdfResistor.h             # Resistor component
    │   └── WdfSeries*.h              # Series circuit components
    └── src/                          # Implementation files
        ├── AudioFilter.cpp
        └── Biquad.cpp
```

## Usage
1. Include the headers in your project:
```cpp
#include "fxobjects/include/AudioFilter.h"
#include "fxobjects/include/Biquad.h"
```

2. Create and use filters:
```cpp
AudioFilter filter;
filter.reset(sampleRate);
filter.setParameters(params);
double output = filter.processAudioSample(input);
```

## Examples
This repository includes IPlug2 implementations of these filter objects as examples. Each example demonstrates how to use the filter classes in a real audio plugin. Examples will be added as we progress through the book chapters.

## Development Setup
1. **Prerequisites**:
   - Microsoft Visual Studio Community 2022
   - iPlug2 framework
   - C++11 or later

2. **Building Examples**:
   - Copy the example project into the `iPlug2-master\Examples` folder
   - Open the `.sln` file in Visual Studio
   - Adjust dependencies as needed
   - Build and run the project

## Dependencies
- C++11 or later
- Standard C++ library
- IPlug2 framework (for examples)
- Microsoft Visual Studio Community 2022

## Building the Examples

### Setup Steps
1. **Clone iPlug2**:
   ```bash
git clone https://github.com/iPlug2/iPlug2.git
```

2. **Copy Example**:
   - Copy your example project into `iPlug2-master\Examples` folder
   - The example should be based on the "IPlugEffect" template
   - Ensure your project maintains the same structure as other examples

3. **Adjust Dependencies**:
   - Open the `.sln` file in Visual Studio
   - In Solution Explorer, right-click on your project
   - Select "Properties"
   - Under C/C++ -> General:
     - Add include paths for your library
     - Ensure iPlug2 paths are correct
   - Under Linker -> General:
     - Add library paths if needed
     - Ensure output paths are set correctly

4. **Common Adjustments**:
   - Add your library's include directory to project includes
   - If using static libraries, add library paths
   - Ensure iPlug2 dependencies are properly linked
   - Check that all required DLLs are in the output directory

### Troubleshooting

1. **Missing Dependencies**:
   - Check that iPlug2 is properly installed
   - Verify all include paths are correct
   - Ensure all required DLLs are present

2. **Build Errors**:
   - Clean and rebuild solution
   - Check for circular dependencies
   - Verify C++11 support is enabled

3. **Plugin Not Loading**:
   - Check if all required DLLs are in the plugin directory
   - Verify plugin format compatibility
   - Check if plugin is properly registered

## Important Notes
- **Homework Sections**: Marked with "homework" comments
- **Original Code**: Most code is unchanged from the book
- **Potential Errors**: Homework sections may contain errors
- **Purpose**: Educational and learning tool

## License
The code in this repository is based on the C++ objects from Will Pirkle's "Designing Audio Effect Plugins in C++" book. According to the book:

"During the course of this book, you will learn both DSP theory and C++ software applications. You have instant access to dozens of C++ objects that implement every signal processing algorithm in the book, and Chapters 9 through 22 will specifically reveal the conversion of the algorithms into C++ code. Each of these chapters will include one to three of these C++ objects. These objects are not tied to any plugin format or operating system. You may easily combine them with other signal processing objects or frameworks, and you have license to use them as you wish for commercial or noncommercial plugins. The resulting products sound fantastic. You will get great satisfaction when you begin inventing your own unique plugins, or implementing cutting-edge audio algorithms as soon as they are published."

This repository is a reorganization of the original code from the book, split into separate files for better readability and maintainability. The code can be used freely for both commercial and non-commercial purposes.
