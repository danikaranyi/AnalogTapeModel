#include "HysteresisProcessor.h"

namespace
{
    constexpr float fBias = 55000.0f;
    constexpr float biasFilterFreq = 24000.0f;
}

HysteresisProcessor::HysteresisProcessor() : ProcessorBase ("HysteresisProcessor")
{
    overSample.reset (new dsp::Oversampling<float> (2, 1, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
    biasFilter.setFreq (biasFilterFreq);
}

void HysteresisProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);

    hProcs[0].setSampleRate ((float) (sampleRate * overSamplingFactor));
    hProcs[1].setSampleRate ((float) (sampleRate * overSamplingFactor));

    overSample->initProcessing (samplesPerBlock);

    biasFilter.prepareToPlay (sampleRate * overSamplingFactor, samplesPerBlock * overSamplingFactor);    

    n[0] = 0;
    n[1] = 0;
}

void HysteresisProcessor::releaseResources()
{
    overSample->reset();

    biasFilter.releaseResources();
}

void HysteresisProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    dsp::AudioBlock<float> block (buffer);
    dsp::AudioBlock<float> osBlock = overSample->processSamplesUp(block);

    float* ptrArray[] = { osBlock.getChannelPointer(0), osBlock.getChannelPointer(1) };
    AudioBuffer<float> osBuffer (ptrArray, 2, static_cast<int> (osBlock.getNumSamples()));

    const auto sineTerm = MathConstants<float>::twoPi * fBias / (float) (getSampleRate() * overSamplingFactor);

    for (int channel = 0; channel < osBuffer.getNumChannels(); ++channel)
    {
        auto* x = osBuffer.getWritePointer (channel);
        for (int samp = 0; samp < osBuffer.getNumSamples(); samp++)
        {
            x[samp] = 27.f * hProcs[channel].process ((float) 1e4 * (x[samp] + 5.0f * sinf (sineTerm * (float) n[channel])));
            
            n[channel]++;
            if ((float) (getSampleRate() * overSamplingFactor * n[channel]) >= 1.0f / fBias)
                n[channel] = 0;
        }
    }
    biasFilter.processBlock (osBuffer, midi);

    overSample->processSamplesDown(block);
}

void HysteresisProcessor::setOverSamplingFactor (String osFactor)
{
    return;
    //@TODO: figure out how to change oversampling factor without breaking everything

    int factor = overSamplingFactor;
    
    if (osFactor == "4x")
    {
        factor = 4; // set overSample factor 4 = 2^2
        overSample.reset (new dsp::Oversampling<float> (2, 2, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
    }
    else if (osFactor == "8x")
    {
        factor = 8; // set overSample factor 8 = 2^3
        overSample.reset (new dsp::Oversampling<float> (2, 3, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
    }
    else if (osFactor == "16x")
    {
        factor = 16; // set overSample factor 16 = 2^4
        overSample.reset (new dsp::Oversampling<float> (2, 4, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
    }

    overSamplingFactor = factor;
    overSample->reset();
    overSample->initProcessing (getBlockSize());
}
