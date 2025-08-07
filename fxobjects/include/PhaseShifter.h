/**
\class PhaseShifter
\ingroup FX-Objects
\brief
The PhaseShifter object implements a six-stage phaser.

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use BiquadParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "IAudioSignalProcessor.h"
#include "AudioFilter.h"
#include "helperfunctions.h"
#include "Lfo.h"


namespace fxobjects
{
    class PhaseShifter : public IAudioSignalProcessor
    {
    public:
        PhaseShifter(void) {
            OscillatorParameters lfoparams = lfo.getParameters();
            lfoparams.waveform = generatorWaveform::kTriangle;	// kTriangle LFO for phaser
        //	lfoparams.waveform = generatorWaveform::kSin;		// kTriangle LFO for phaser
            lfoparams.amplitude_fac = 1.0;
            lfo.setParameters(lfoparams);
    
            AudioFilterParameters params = apf[0].getParameters();
            params.algorithm = filterAlgorithm::kAPF1; // can also use 2nd order
            // params.Q = 0.001; use low Q if using 2nd order APFs
    
            for (uint32_t i = 0; i < PHASER_STAGES; i++)
            {
                apf[i].setParameters(params);
            }
        }	/* C-TOR */
    
        ~PhaseShifter(void) {}	/* D-TOR */
    
    public:
        /** reset members to initialized state */
        virtual bool reset(double _sampleRate)
        {
            // --- reset LFO
            lfo.reset(_sampleRate);
    
            // --- reset APFs
            for (uint32_t i = 0; i < PHASER_STAGES; i++){
                apf[i].reset(_sampleRate);
            }
    
            return true;
        }
    
        /** process autio through phaser */
        /**
        \param xn input
        \return the processed sample
        */
        virtual double processAudioSample(double xn)
        {
            SignalGenData lfoData = lfo.renderAudioOutput();
    
            // --- create the bipolar modulator value
            double lfoValue = lfoData.normalOutput;
            if (parameters.quadPhaseLFO)
                lfoValue = lfoData.quadPhaseOutput_pos;
    
            double depth = parameters.lfoDepth_Pct / 100.0;
            double modulatorValue = lfoValue*depth;
    
            // --- calculate modulated values for each APF; note they have different ranges
            AudioFilterParameters params = apf[0].getParameters();
            params.fc = doBipolarModulation(modulatorValue, apf0_minF, apf0_maxF);
            apf[0].setParameters(params);
    
            params = apf[1].getParameters();
            params.fc = doBipolarModulation(modulatorValue, apf1_minF, apf1_maxF);
            apf[1].setParameters(params);
    
            params = apf[2].getParameters();
            params.fc = doBipolarModulation(modulatorValue, apf2_minF, apf2_maxF);
            apf[2].setParameters(params);
    
            params = apf[3].getParameters();
            params.fc = doBipolarModulation(modulatorValue, apf3_minF, apf3_maxF);
            apf[3].setParameters(params);
    
            params = apf[4].getParameters();
            params.fc = doBipolarModulation(modulatorValue, apf4_minF, apf4_maxF);
            apf[4].setParameters(params);
    
            params = apf[5].getParameters();
            params.fc = doBipolarModulation(modulatorValue, apf5_minF, apf5_maxF);
            apf[5].setParameters(params);
    
            // --- calculate gamma values
            double gamma1 = apf[5].getG_value();
            double gamma2 = apf[4].getG_value() * gamma1;
            double gamma3 = apf[3].getG_value() * gamma2;
            double gamma4 = apf[2].getG_value() * gamma3;
            double gamma5 = apf[1].getG_value() * gamma4;
            double gamma6 = apf[0].getG_value() * gamma5;
    
            // --- set the alpha0 value
            double K = parameters.intensity_Pct / 100.0;
            double alpha0 = 1.0 / (1.0 + K*gamma6);
    
            // --- create combined feedback
            double Sn = gamma5*apf[0].getS_value() + gamma4*apf[1].getS_value() + gamma3*apf[2].getS_value() + gamma2*apf[3].getS_value() + gamma1*apf[4].getS_value() + apf[5].getS_value();
    
            // --- form input to first APF
            double u = alpha0*(xn + K*Sn);
    
            // --- cascade of APFs (could also nest these in one massive line of code)
            double APF1 = apf[0].processAudioSample(u);
            double APF2 = apf[1].processAudioSample(APF1);
            double APF3 = apf[2].processAudioSample(APF2);
            double APF4 = apf[3].processAudioSample(APF3);
            double APF5 = apf[4].processAudioSample(APF4);
            double APF6 = apf[5].processAudioSample(APF5);
    
            // --- sum with -3dB coefficients
            //	double output = 0.707*xn + 0.707*APF6;
    
            // --- sum with National Semiconductor design ratio:
            //	   dry = 0.5, wet = 5.0
            // double output = 0.5*xn + 5.0*APF6;
            // double output = 0.25*xn + 2.5*APF6;
            double output = 0.125*xn + 1.25*APF6;
    
            return output;
        }
    
        /** return false: this object only processes samples */
        virtual bool canProcessAudioFrame() { return false; }
    
        /** get parameters: note use of custom structure for passing param data */
        /**
        \return PhaseShifterParameters custom data structure
        */
        PhaseShifterParameters getParameters() { return parameters; }
    
        /** set parameters: note use of custom structure for passing param data */
        /**
        \param PhaseShifterParameters custom data structure
        */
        void setParameters(const PhaseShifterParameters& params)
        {
            // --- update LFO rate
            if (params.lfoRate_Hz != parameters.lfoRate_Hz || params.lfoAmplitude_fac != parameters.lfoAmplitude_fac)
            {
                OscillatorParameters lfoparams = lfo.getParameters();
                lfoparams.frequency_Hz = params.lfoRate_Hz;
                lfoparams.amplitude_fac = params.lfoAmplitude_fac;
                lfo.setParameters(lfoparams);
            }
    
            // --- save new
            parameters = params;
        }
    protected:
        PhaseShifterParameters parameters;  ///< the object parameters
        AudioFilter apf[PHASER_STAGES];		///< six APF objects
        LFO lfo;							///< the one and only LFO
    };
} // namespace fxobjects
