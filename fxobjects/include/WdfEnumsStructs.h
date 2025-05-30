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
} // namespace fxobjects