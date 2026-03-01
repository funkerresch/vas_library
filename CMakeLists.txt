cmake_minimum_required(VERSION 3.15)

project(vas_library
    VERSION 0.1
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_PD_EXTERNALS "Build Pure Data externals" ON)

# ==========================================================
# Add pffft submodule
# ==========================================================

set(USE_BENCH_FFTW   OFF CACHE BOOL "" FORCE)
set(USE_BENCH_GREEN  OFF CACHE BOOL "" FORCE)
set(USE_BENCH_KISS   OFF CACHE BOOL "" FORCE)
set(USE_BENCH_POCKET OFF CACHE BOOL "" FORCE)
set(USE_BENCH_MKL    OFF CACHE BOOL "" FORCE)
set(USE_FFTPACK      OFF CACHE BOOL "" FORCE)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pffft/CMakeLists.txt")
    add_subdirectory(pffft)
else()
    message(FATAL_ERROR "pffft submodule not found. Did you run git submodule update --init --recursive?")
endif()

# ==========================================================
# Add libmysofa submodule
# ==========================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libmysofa/CMakeLists.txt")
    # Disable tests inside libmysofa
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(libmysofa)
else()
    message(FATAL_ERROR "libmysofa submodule not found. Did you init submodules?")
endif()

# ==========================================================
# Collect ALL library sources
# ==========================================================

file(GLOB_RECURSE VAS_C_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
)

file(GLOB_RECURSE VAS_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

file(GLOB_RECURSE VAS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

# Remove files inside source/notInUse
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/vas_util_vectorIntrinsics.*")
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/vas_pitch.*")
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/vas_fixedFirChannel.*")
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/vas_iir_2highpass2lowpass.*")
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/vas_iir_lowpass.*")
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/vas_iir_highpass.*")
list(FILTER VAS_C_SOURCES EXCLUDE REGEX ".*/oldNotInUseAnymore/.*")

list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/vas_util_vectorIntrinsics.*")
list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/vas_pitch.*")
list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/vas_fixedFirChannel.*")
list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/vas_iir_lowpass.*")
list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/vas_iir_highpass.*")
list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/vas_iir_2highpass2lowpass.*")
list(FILTER VAS_HEADERS EXCLUDE REGEX ".*/oldNotInUseAnymore/.*")

add_library(vas_library STATIC
    ${VAS_C_SOURCES}
    ${VAS_CPP_SOURCES}
    ${VAS_HEADERS}
)

target_compile_definitions(vas_library
    PUBLIC PUREDATA 
    VAS_INPUTVECTOR_USE_FLOAT
    VAS_OUTPUTVECTOR_USE_FLOAT
)

target_include_directories(vas_library
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# ==========================================================
# Link pffft and libmysofa
# ==========================================================

target_link_libraries(vas_library
    PUBLIC
        #pffft
        #mysofa
)

# ==========================================================
# Apple Accelerate
# ==========================================================

if(APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    if(ACCELERATE_FRAMEWORK)
        target_link_libraries(vas_library PUBLIC ${ACCELERATE_FRAMEWORK})
    endif()
endif()

# ==========================================================
# Pure Data Externals
# ==========================================================

if(BUILD_PD_EXTERNALS)

    add_library(rwa_binauralsimple SHARED
        examples/PureData/rwa_binauralsimple~.c
    )

    target_compile_definitions(rwa_binauralsimple
        PRIVATE PUREDATA
    )

    target_include_directories(rwa_binauralsimple
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    target_link_libraries(rwa_binauralsimple
        PRIVATE vas_library
    )

    set_target_properties(rwa_binauralsimple PROPERTIES
        PREFIX ""
        SUFFIX ".pd_darwin"
    )

    if(APPLE)
        set_target_properties(rwa_binauralsimple PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
        )
    endif()

endif()