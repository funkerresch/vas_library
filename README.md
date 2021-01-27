# vas_library
A library for (dynamic) binaural (room) synthesis (and related fir filtering stuff)

Virtual Acoustic Space (VAS) is a C library for binaural 3D sound reproduction over headphones. In addition to standard objects for Head Related Transfer Function (HRTF)-based dynamic binaural synthesis, delays for simulating reflections, and convolution-based filters for headphone equalization/reverberation, the VAS Library implements a data reduction for binaural room impulse responses (BRIRs).

The VAS library implements a classic, equal partitioned fast convolution algorithm. For the necessary dynamic filter change in realtime, at the moment of the angle change between source and listener, both the impulse response of the old and the target angle are calculated and a crossfade is performed in the time domain. Depending on the audio material, this crossfade must be between 256 and 1024 samples at 44.1 kHz; with 1024 samples, playback is (in my experience) always free of artifacts. 
By splitting IRs in two or more parts and calculting later parts with a greater partition size, a more CPU friendly non-equal partition approach is also realised. This is already done
for the Unity Plugin, a Pure Data version will follow soon.

The raytracing/mirror source plugin is currently rather a proof of concept than a finished plugin. Material characteristics, Air arbsorption etc. are implemented in a very simple manner and
far from physically accurate. Also the number of reflections is rather limited

The male speech audio file used in most examples is from:
http://dx.doi.org/10.14279/depositonce-8536

Usage

Documentation is not complete, most work has been done for the Unity and Pure Data examples, Max/MSP examples are broken right now (soon to be fixed).
Helpfiles for the Pure Data objects vas_binaural~, vas_reverb~ and vas_hpcomp~ are in Examples/PureData/doc

In the Examples folder are implementations for dynamic binaural (room) synthesis for Pure Data and Unity. Pd examples are XCode Projects for OSX. 
The crossplatform makefile in Examples/PureData/vas_pd_linux should work for linux, osx and windows (only tested for linux).
For Unity you will find both, Visual Studio and XCode Projects. 

In Pure Data, the HRTF/BRIR should be placed in the same folder where the Pd Patch resides. Then you can simply pass the HRTF/BRIR name as the first argument then followed by partition size as second argument and an offset for the IR. Useful partition sizes are 512 or 1024 samples for BRIR Synthesis and up to 64 Samples for HRTF Synthesis.
For the Unity Plugin on, put the BRIR/HRTF files inside the 'StreamingAssets' Folder. 

A sofa to vas textformat converter is in the matlab folder. Sofa support will come back soon

To do next:

Include sofa support again.
Redo MaxMSP objects.
Material Characteristics for the Unity spatializer.
Change fft framework for non-Apple OSs.





