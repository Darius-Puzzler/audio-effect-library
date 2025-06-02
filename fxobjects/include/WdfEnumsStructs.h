/**
\enum wdfComponent
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the wdfComponent type

- enum class wdfComponent { R, L, C, seriesLC, parallelLC, seriesRL, parallelRL, seriesRC, parallelRC };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

namespace fxobjects {

	enum class wdfComponent {
		R,
		L,
		C,
		seriesLC,
		parallelLC,
		seriesRL,
		parallelRL,
		seriesRC,
		parallelRC
	};
	
	struct WdfComponentInfo
	{
		WdfComponentInfo() { }
	
		WdfComponentInfo(wdfComponent _componentType, double value1 = 0.0, double value2 = 0.0)
		{
			componentType = _componentType;
			if (componentType == wdfComponent::R)
				R = value1;
			else if (componentType == wdfComponent::L)
				L = value1;
			else if (componentType == wdfComponent::C)
				C = value1;
			else if (componentType == wdfComponent::seriesLC || componentType == wdfComponent::parallelLC)
			{
				L = value1;
				C = value2;
			}
			else if (componentType == wdfComponent::seriesRL || componentType == wdfComponent::parallelRL)
			{
				R = value1;
				L = value2;
			}
			else if (componentType == wdfComponent::seriesRC || componentType == wdfComponent::parallelRC)
			{
				R = value1;
				C = value2;
			}
		}
	
		double R = 0.0; ///< value of R component
		double L = 0.0;	///< value of L component
		double C = 0.0;	///< value of C component
		wdfComponent componentType = wdfComponent::R; ///< selected component type
	};
	
	struct WDFParameters
	{
		WDFParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		WDFParameters& operator=(const WDFParameters& params)
		{
			if (this == &params)
				return *this;

			fc = params.fc;
			Q = params.Q;
			boostCut_dB = params.boostCut_dB;
			frequencyWarping = params.frequencyWarping;
			return *this;
		}

		// --- individual parameters
		double fc = 100.0;				///< filter fc
		double Q = 0.707;				///< filter Q
		double boostCut_dB = 0.0;		///< filter boost or cut in dB
		bool frequencyWarping = true;	///< enable frequency warping
	};
} // namespace fxobjects