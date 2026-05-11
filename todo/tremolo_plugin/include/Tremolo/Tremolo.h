#pragma once

namespace tremolo {
class Tremolo
{
public:
  enum class LfoWaveform : size_t
  {
    sine = 0,
    triangle = 1,
  };
  Tremolo()
  {
    for (auto& lfo : lfos)
    {
      lfo.setFrequency(5.f, true);
    }

  }

  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    // ProcessSpec przygotowuje nam parametry dla innego elementu który trzeba prepare()
    const juce::dsp::ProcessSpec processSpec
    {
      .sampleRate = sampleRate,
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = 1u,
    };
    for (auto& lfo : lfos)
    {
      lfo.prepare(processSpec);
    }
  }
  //funkcja ustawiajaca aktualny typ waveform
  void setLfoWaveform(LfoWaveform waveform)
  {
    jassert(waveform == LfoWaveform::sine || waveform == LfoWaveform::triangle); //jesli to nie bedzie zadne z powyzszych, program sie zatrzyma kontrolowanie, dziala tylko w debug
    lfoToSet = waveform;
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    // for each frame
    updateLfoWaveform(); //aktualizuje typ waveform tylko przed procesowaniem nastepnego bufora, co jest bezpieczniejszym rozwiazaniem

    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {

      // obliczanie jednej probki na jedna ramke
      const auto lfoValue = getNextLfoValue();
      //0 jako input, bo jest to wartosc poczatkowa, prawdopodobnie dziala jak skladowa stala

      constexpr auto modulationDepth = 0.5f;
      const auto modulationValue = modulationDepth * lfoValue + 1.f;

      //TODO: Możesz spróbować dodawania: outputSample=inputSample+lfoValue*modulationDepth

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        //  modulate the sample
        // probka jaka uslyszymy na wyjsciu efektu:
        const auto outputSample = inputSample * modulationValue;


        // set the output sample
        buffer.setSample(channelIndex, frameIndex, outputSample);
           }
    }
  }
  // wszystko co wczesniej rozpoczelismy, musimy zresetowac/zwolnic zasoby
  void reset() noexcept
  {
    for (auto& lfo : lfos)
    {
      lfo.reset();
    }
  }

private:

  //funkcja przebiegu trojkatnego
  static float triangle(float phase)
  {
    const auto ft = phase/juce::MathConstants<float>::twoPi;
    return 4.f*std::abs(ft-std::floor(ft+0.5f))-1.f;
  }

  float getNextLfoValue()
  {
    return lfos[juce::toUnderlyingType(currentLfo)].processSample(0.f);
  }

  void updateLfoWaveform()
  {
    if (currentLfo != lfoToSet) currentLfo = lfoToSet;
  }

  //funkcja lambda:
  //[] - przechwytywane zmienne zewnetrzene (jesli takich potrzebujemy)
  //() - parametry (jak w normalnej funkcji)
  //{} - ciało funkcji

  //kontener zawierajacy typy lfo
  std::array<juce::dsp::Oscillator<float>, 2u> lfos
  {
    juce::dsp::Oscillator<float>{[](auto phase){return std::sin(phase);}},
    juce::dsp::Oscillator<float>{triangle},
  };

  LfoWaveform currentLfo = LfoWaveform::sine; //flaga umozliwiajaca okreslenie jaki typ lfo jest wybrany
  LfoWaveform lfoToSet = currentLfo;
};
}  // namespace tremolo
