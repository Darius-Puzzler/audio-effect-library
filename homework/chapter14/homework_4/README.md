# Ducking Delay Effect

## Overview
This is an implementation of a ducking delay effect inspired by the classic TC-2290 Dynamic Delay. The effect automatically reduces the delay level when the input signal exceeds a threshold, allowing the dry signal to be more prominent during loud passages.

## Features
- **Dual Delay Lines**: Independent left and right channel delays
- **Envelope Follower**: Tracks input signal level for ducking control
- **Smooth Parameter Transitions**: Prevents audio artifacts during parameter changes
- **Stereo Processing**: Full stereo in/out processing

## Parameters

### Main Controls
- **Gain**: Overall output level (0-100%)
- **Sidechain Gain**: Controls the input level to the envelope follower (0-100%)

### Delay Controls
- **Left/Right Delay Time**: Delay time in milliseconds for each channel
- **Left/Right Feedback**: Feedback amount for each delay line (0-100%)

### Envelope Follower
- **Attack**: Envelope attack time in milliseconds
- **Release**: Envelope release time in milliseconds
- **Threshold**: Level at which ducking begins (in dB)
- **Sensitivity**: How aggressively the effect responds to input level changes

### Wet/Dry Mix
- **Wet Min**: Minimum wet signal level when ducking is active (in dB)
- **Wet Max**: Maximum wet signal level when no ducking is applied (in dB)

## Implementation Details
- Uses a stereo audio delay with independent left/right processing
- Envelope follower detects input signal level to control ducking
- Smooth parameter interpolation to prevent audio artifacts
- All parameters are smoothed to prevent zipper noise
- Processing is done in blocks for efficiency

## Technical Notes
- Sample rate and block size are configurable via the `reset()` method
- The effect processes audio in blocks rather than individual samples
- All parameters are smoothed using logarithmic interpolation
- The envelope follower operates in dB for accurate level detection
- The wet/dry mix is controlled in the linear domain for smooth transitions

## Usage
1. Set the desired delay times and feedback amounts
2. Adjust the threshold to set when ducking should begin
3. Use the wet min/max controls to set the dynamic range of the effect
4. Fine-tune the envelope response with attack and release controls
5. Adjust the sensitivity to control how aggressively the effect responds to input level changes
