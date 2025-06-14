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

    const unsigned int TLD_AUDIO_DETECT_MODE_PEAK = 0;
    const unsigned int TLD_AUDIO_DETECT_MODE_MS = 1;
    const unsigned int TLD_AUDIO_DETECT_MODE_RMS = 2;
    const double TLD_AUDIO_ENVELOPE_ANALOG_TC = -0.99967234081320612357829304641019; // ln(36.7%)

	//-----------------------------------------------------------------------------
	/// @name Audio Detector Constants
	//-----------------------------------------------------------------------------
	/** @AudioDetectorConstants
	\ingroup Constants-Enums @{*/
	// ---
	const uint32_t ENVELOPE_DETECT_MODE_PEAK = 0;	///< |x|
	const uint32_t ENVELOPE_DETECT_MODE_MS = 1;		///< (1/N)|x|^2
	const uint32_t ENVELOPE_DETECT_MODE_RMS = 2;	///< SQRT((1/N)|x|^2)
	const uint32_t ENVELOPE_DETECT_MODE_NONE = 3;	///< not used

	const float ENVELOPE_DIGITAL_TC = -4.6051701859880913680359829093687;///< ln(1%)
	const float ENVELOPE_ANALOG_TC = -1.0023934309275667804345424248947; ///< ln(36.7%)
	/** @} */

	/** @GUITiming
	\ingroup Constants-Enums @{*/
	// ---
	const float GUI_METER_UPDATE_INTERVAL_MSEC = 50.f;	///< repaint interval; larger = slower
	const float GUI_METER_MIN_DB = -60.f;				///< min GUI value in dB
	/** @} */

	/** \ingroup Constants-Enums */
#define FLT_EPSILON_PLUS      1.192092896e-07        ///< /* smallest such that 1.0+FLT_EPSILON != 1.0 */
/** \ingroup Constants-Enums */
#define FLT_EPSILON_MINUS    -1.192092896e-07        ///< /* smallest such that 1.0-FLT_EPSILON != 1.0 */
/** \ingroup Constants-Enums */
#define FLT_MIN_PLUS          1.175494351e-38        ///< /* min positive value */
/** \ingroup Constants-Enums */
#define FLT_MIN_MINUS        -1.175494351e-38        ///< /* min negative value */

// --- for math.h constants
#define _MATH_DEFINES_DEFINED

	/**
	\enum smoothingMethod
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the smoothing type

	- enum class smoothingMethod { kLinearSmoother, kLPFSmoother };

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	enum class smoothingMethod { kLinearSmoother, kLPFSmoother };

	/**
	\enum taper
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the control taper

	- enum class taper { kLinearTaper, kLogTaper, kAntiLogTaper, kVoltOctaveTaper };

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	enum class taper { kLinearTaper, kLogTaper, kAntiLogTaper, kVoltOctaveTaper };

	/**
	\enum meterCal
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set meter calibration.

	- enum class meterCal { kLinearMeter, kLogMeter };

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	enum class meterCal { kLinearMeter, kLogMeter };

	/**
	\enum controlVariableType
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the control's behavior; this tells the PluginParameter object
	how to interpret the control information (e.g. as float versus int).\n\n
	Note that you can set a PluginParameter as kNonVariableBoundControl to indicate that it is not bound to any
	variable.

	- enum class controlVariableType { kFloat, kDouble, kInt, kTypedEnumStringList, kMeter, kNonVariableBoundControl };

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	enum class controlVariableType { kFloat, kDouble, kInt, kTypedEnumStringList, kMeter, kNonVariableBoundControl };


	/**
	\enum boundVariableType
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the control's linked variable datatype (for automatic variable binding).

	- enum boundVariableType { kFloat, kDouble, kInt, kUInt };

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	enum class boundVariableType { kFloat, kDouble, kInt, kUInt };

} // namespace fxobjects