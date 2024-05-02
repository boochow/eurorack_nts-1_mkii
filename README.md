# eurorack_nts-1_mkii
This is the NTS-1 mkII port of Braids, the Mutable Instrument's macro oscillator.

Due to a file size limitation of 48Kb for the NTS-1 mkII oscillator module, the ocillators are divided into five different files:

- VA
  Analog oscillators, including a ring modulator and saw swarm.

- FM
  Includes Waveshaping, voice/formant synthesis, additive synthesis, and FM synthesis.

- RS
  Resonator-based synthesis, featureing pluck, string, wind, bell, and percussion sounds.

- WT
  Wavetable sysnthesis.

- NZ
  Noise generators.

## Features

All oscillators share the following parameters:

- **Timbre**(knob A): Modifies the waveform.
- **Color**(knob B): Also modifies the waveform, but in a different way.


- **Shape**: Selects one of the oscillator models.
- **MTgt**: Chooses the modulation target parameter from timbre, color, or FM level.
- **MDly**: Sets the delay time between a note-on event and the parameter modulation.
- **FMLv**: Adjusts the amount of frequency modulation influenced by the audio input.


- **Ptch**: Used for fine-tuning of the oscillator's pitch.
- **Bits**: Selects the quantization bit rate from options of 2, 3, 4, 6, 8, 12, or 16bits.
- **Rate**: Chooses the sampling rate from 4, 6, 8, 16, 24, or 48KHz.
  
