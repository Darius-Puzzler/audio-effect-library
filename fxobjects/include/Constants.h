#pragma once

namespace fxobjects {
    
    // Math constants
    const double kPi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;
    const double kTwoPi = 2.0 * kPi;
    const double kSqrtTwo = 1.41421356237309504880168872420969807856967187537694807317667973799;  // √2
    
    const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
    const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */
    const double kMinFilterFrequency = 20.0;
    const double kMaxFilterFrequency = 20480.0; // 10 octaves above 20 Hz
    const double ARC4RANDOMMAX = 4294967295.0;  // (2^32 - 1)
    
    constexpr int NEGATIVE = 0;
    constexpr int POSITIVE = 1;

} // namespace fxobjects