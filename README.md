# vas_library
A library for (dynamic) binaural (room) synthesis (and related fir filtering stuff)

Virtual Acoustic Spaces (VAS) is a C library for binaural 3D sound reproduction over headphones. In addition to standard objects for Head Related Transfer Function (HRTF)-based dynamic binaural synthesis, delays for simulating reflections, and convolution-based filters for headphone equalization/reverberation, the VAS Library implements an algorithm for analyzing and compressing binaural room impulse responses (BRIRs).
The VAS library implements a classic, equal partitioned fast convolution algorithm. For the necessary dynamic filter change in realtime, at the moment of the angle change between source and listener, both the impulse response of the old and the target angle are calculated and a crossfade is performed in the time domain. Depending on the audio material, this crossfade must be between 256 and 1024 samples at 44.1 kHz; with 1024 samples, playback is (in my experience) always free of artifacts. Depending on the source material shorter crossfade times are possible. 


Usage

In the Examples folder are implementations for dynamic binaural (room) synthesis for MaxMsp, Pure Data, JUCE and Unity. Max and Pd examples are XCode Projects for OSX, JUCE is of course multiplatform and for Unity you will find both, Visual Studio and XCode Projects

In Max as in Pure Data, the HRTF/BRIR should be placed in the same folder as the Max/Pd Patch. Then you can simply pass the HRTF/BRIR name as the first argument then followed by partition size as second argument. Useful partition sizes are 512 or 1024 samples for BRIR Synthesis and up to 64 Samples for HRTF Synthesis.
For the Unity Plugin on OSX, put the BRIR files inside the Plugin Bundle in MacOS (Show Package Contents -> Contents/MacOS). Since they are quite large I excluded them from the Repository; they can be downloaded from xxx.xxx.xx
Because Unity Audio Plugins only accept floating point arguments, the BRIR's are choosen by numbers from 0 to 10 for the parameter "Select IR".
Under Windows, place the BRIR/HRTF in the same folder as the Plugin. Unfortunatly this works not for Windows Builds yet because Unity seems to ignore the Textfile.


Known Issues

Unity seems to have some kind of upper memory limit for plugins (this has to be investigated further, for now it's just an assumption); while all other example implementations work perfect with large BRIR files, the Unity Editor produces weird noise sometimes if several instances are loaded into one Audio Mixer. However, a standalone build (at least under OSX) works always fine.




