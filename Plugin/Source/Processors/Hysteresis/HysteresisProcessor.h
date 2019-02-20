#ifndef HYSTERESISPROCESSOR_H_INCLUDED
#define HYSTERESISPROCESSOR_H_INCLUDED

#include "../ProcessorBase.h"
#include "HysteresisProcessing.h"
#include "Bias Filter/BiasFilter.h"

class HysteresisProcessor : public ProcessorBase
{
public:
    HysteresisProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiBuffer) override;

    double getTailLengthSeconds() const override { return overSample->getLatencyInSamples() * getSampleRate(); }

    void setOverSamplingFactor (String osFactor);

private:
    HysteresisProcessing hProcs[2];
    std::unique_ptr<dsp::Oversampling<float>> overSample;
    BiasFilter biasFilter;

    int overSamplingFactor = 8;
    int n[2] = { 0, 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HysteresisProcessor)
};

#endif //HYSTERESISPROCESSOR_H_INCLUDED
