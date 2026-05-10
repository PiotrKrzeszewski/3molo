#pragma once

namespace tremolo {
class Tremolo {
public:
  Tremolo()
  {
    lfo.setFrequency(440.f,true); //ustawienie czestotliwosci podczas inicjalizacji tremolo
  }

  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    // ProcessSpec przygotowuje nam parametry dla innego elementu który trzeba prepare()
    const juce::dsp::ProcessSpec processSpec
    {
      .sampleRate = sampleRate,
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = 1u,
    };
    lfo.prepare(processSpec);
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      // obliczanie jednej probki na jedna ramke
      const auto lfoValue = lfo.processSample(0.f);
      //0 jako input, bo jest to wartosc poczatkowa, prawdopodobnie dziala jak skladowa stala
      // TODO: calculate the modulation value

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        // TODO: modulate the sample
        // probka jaka uslyszymy na wyjsciu efektu:
        const auto outputSample = 0.1f*lfoValue; //0.1 dla bezpieczenstwa sluchawek


        // set the output sample
        buffer.setSample(channelIndex, frameIndex, outputSample);
      }
    }
  }
  // wszystko co wczesniej rozpoczelismy, musimy zresetowac/zwolnic zasoby
  void reset() noexcept
  {
    lfo.reset();
  }

private:
  // Dodanie oscylatora do mojego tremolo, mapowanie faza-próbka
  juce::dsp::Oscillator<float> lfo{ [](auto phase){return std::sin(phase);}};

  //funkcja lambda:
  //[] - przechwytywane zmienne zewnetrzene (jesli takich potrzebujemy)
  //() - parametry (jak w normalnej funkcji)
  //{} - ciało funkcji

};
}  // namespace tremolo
