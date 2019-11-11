# vas_library
A library for (dynamic) binaural (room) synthesis (and related fir filtering stuff)

Virtual Acoustic Space (VAS) is a C library for binaural 3D sound reproduction over headphones. In addition to standard objects for Head Related Transfer Function (HRTF)-based dynamic binaural synthesis, delays for simulating reflections, and convolution-based filters for headphone equalization/reverberation, the VAS Library implements an algorithm for analyzing and compressing binaural room impulse responses (BRIRs).

The VAS library implements a classic, equal partitioned fast convolution algorithm. For the necessary dynamic filter change in realtime, at the moment of the angle change between source and listener, both the impulse response of the old and the target angle are calculated and a crossfade is performed in the time domain. Depending on the audio material, this crossfade must be between 256 and 1024 samples at 44.1 kHz; with 1024 samples, playback is (in my experience) always free of artifacts. 

Several vas_dynamicFirFilters with different partition sizes can be combined in order to realize
a non-uniformly partitioned convolution. This is done for the Unity spatializer plugin in the BRIR example.

Usage

Documentation is not complete, most work has been done for the Unity examples, Max/MSP examples are broken right now (soon to be fixed). Pure Data externals are working, but helpfiles
are still missing.

In the Examples folder are implementations for dynamic binaural (room) synthesis for Pure Data,  and Unity. M Pd examples are XCode Projects for OSX and for Unity you will find both, Visual Studio and XCode Projects. Linux for Pd is almost ready.

In Pure Data, the HRTF/BRIR should be placed in the same folder where the Pd Patch resides. Then you can simply pass the HRTF/BRIR name as the first argument then followed by partition size as second argument. Useful partition sizes are 512 or 1024 samples for BRIR Synthesis and up to 64 Samples for HRTF Synthesis.
For the Unity Plugin on, put the BRIR/HRTF files inside the 'StreamingAssets' Folder. 

To do next:

Include sofa support again.
Redo MaxMSP objects.
Material Characteristics for the Unity spatializer.
Change fft framework for non-Apple OSs.





