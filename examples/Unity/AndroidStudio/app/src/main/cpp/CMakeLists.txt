# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)


# Declares and names the project.

project("vas_binaural")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(UNITYPLUGINPATH "../../../../..")
set(VASPATH "../../../../../../../source")
set(KISSFFT "../../../../../../../kissfft")
set(KISSFFTTOOLS "../../../../../../../kissfft/tools")
include_directories(${UNITYPLUGINPATH})
include_directories(${VASPATH})
include_directories(${KISSFFT})
include_directories(${KISSFFTTOOLS})

add_definitions(-D__ANDROID__)
add_definitions(-DPLATFORM_LINUX)


add_library( # Sets the name of the library.
        AudioPlugin_VAS_Binaural

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).

        ${UNITYPLUGINPATH}/VasUnitySpatializer.cpp
        ${UNITYPLUGINPATH}/AudioPluginUtil.cpp
        ${VASPATH}/vas_delay.c
        ${VASPATH}/vas_delay_crossfade.c
        ${VASPATH}/vas_ringBuffer.c
        ${VASPATH}/vas_delayTap.c
        ${VASPATH}/vas_delayTap_crossfade.c
        ${VASPATH}/vas_iir_biquad.c
        ${VASPATH}/vas_fir_binauralReflection1.c
        ${VASPATH}/vas_dynamicFirChannel.c
        ${VASPATH}/vas_fir_binaural.c
        ${VASPATH}/vas_fir_list.c
        ${VASPATH}/vas_fir.c
        ${VASPATH}/vas_mem.c
        ${VASPATH}/vas_util.c
      #  ${VASPATH}/vas_util_vectorIntrinsics.c
        ${KISSFFTTOOLS}/kiss_fftr.c
        ${KISSFFT}/kiss_fft.c)

target_compile_options(AudioPlugin_VAS_Binaural PRIVATE
        "$<$<CONFIG:RELEASE>:-Ofast>"
        "$<$<CONFIG:DEBUG>:-Ofast>"
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        AudioPlugin_VAS_Binaural

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )