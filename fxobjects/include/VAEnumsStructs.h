/**
\enum vaFilterAlgorithm
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the virtual analog filter algorithm

- enum class vaFilterAlgorithm { kLPF1, kHPF1, kAPF1, kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

namespace fxobjects
{    
    enum class vaFilterAlgorithm {
        kLPF1, kHPF1, kAPF1, kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS
    };
    
    struct ZVAFilterParameters
    {
        ZVAFilterParameters() {}
        /** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
        ZVAFilterParameters& operator=(const ZVAFilterParameters& params)	// need this override for collections to work
        {
            if (this == &params)
                return *this;
    
            filterAlgorithm = params.filterAlgorithm;
            fc = params.fc;
            Q = params.Q;
            filterOutputGain_dB = params.filterOutputGain_dB;
            enableGainComp = params.enableGainComp;
            matchAnalogNyquistLPF = params.matchAnalogNyquistLPF;
            selfOscillate = params.selfOscillate;
            enableNLP = params.enableNLP;
            return *this;
        }
    
        // --- individual parameters
        vaFilterAlgorithm filterAlgorithm = vaFilterAlgorithm::kSVF_LP;	///< va filter algorithm
        double fc = 1000.0;						///< va filter fc
        double Q = 0.707;						///< va filter Q
        double filterOutputGain_dB = 0.0;		///< va filter gain (normally unused)
        bool enableGainComp = false;			///< enable gain compensation (see book)
        bool matchAnalogNyquistLPF = false;		///< match analog gain at Nyquist
        bool selfOscillate = false;				///< enable selfOscillation
        bool enableNLP = false;					///< enable non linear processing (use oversampling for best results)
    };
} // namespace fxobjects