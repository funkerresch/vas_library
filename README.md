# vas_library
A library for (dynamic) binaural (room) synthesis (and related fir filtering stuff)

Virtual Acoustic Spaces (VAS) is a C library for binaural 3D sound reproduction over headphones. In addition to standard objects for Head Related Transfer Function (HRTF)-based dynamic binaural synthesis, delays for simulating reflections, and convolution-based filters for headphone equalization/reverberation, the VAS Library implements an algorithm for analyzing and compressing binaural room impulse responses (BRIRs).
The VAS library implements a classic, equal partitioned fast convolution algorithm. For the necessary dynamic filter change in realtime, at the moment of the angle change between source and listener, both the impulse response of the old and the target angle are calculated and a crossfade is performed in the time domain. Depending on the audio material, this crossfade must be between 256 and 1024 samples at 44.1 kHz; with 1024 samples, playback is (in my experience) always free of artifacts. Depending on the source material shorter crossfade times are possible. 



Usage

In the Examples folder are implementations for dynamic binaural (room) synthesis for MaxMsp, PureData, JUCE and Unity.



