//
//  vas_util_vectorIntrinsics.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 29.10.19.
//

#include "vas_util_vectorIntrinsics.h"

void __cpuid(int* cpuinfo, int info)
{
    __asm__ __volatile__(
        "xchg %%ebx, %%edi;"
        "cpuid;"
        "xchg %%ebx, %%edi;"
        :"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
        :"0" (info)
    );
}

unsigned long long _xgetbv(unsigned int index)
{
    unsigned int eax, edx;
    __asm__ __volatile__(
        "xgetbv;"
        : "=a" (eax), "=d"(edx)
        : "c" (index)
    );
    return ((unsigned long long)edx << 32) | eax;
}

bool vas_util_vectorIntrinsics_supportsAVX(void)
{
    bool avxSupportted = false;

    int cpuinfo[4];
    __cpuid(cpuinfo, 1);
    
    avxSupportted = cpuinfo[2] & (1 << 28) || false;
    bool osxsaveSupported = cpuinfo[2] & (1 << 27) || false;
    if (osxsaveSupported && avxSupportted)
    {
        // _XCR_XFEATURE_ENABLED_MASK = 0
        unsigned long long xcrFeatureMask = _xgetbv(0);
        avxSupportted = (xcrFeatureMask & 0x6) == 0x6;
    }

    return avxSupportted;
}

/*bool vas_util_vectorIntrinsics_supportsAVX()
{
    bool sseSupportted = false;
    bool sse2Supportted = false;
    bool sse3Supportted = false;
    bool ssse3Supportted = false;
    bool sse4_1Supportted = false;
    bool sse4_2Supportted = false;
    bool sse4aSupportted = false;
    bool sse5Supportted = false;
    bool avxSupportted = false;

    int cpuinfo[4];
    __cpuid(cpuinfo, 1);

    // Check SSE, SSE2, SSE3, SSSE3, SSE4.1, and SSE4.2 support
    sseSupportted        = cpuinfo[3] & (1 << 25) || false;
    sse2Supportted        = cpuinfo[3] & (1 << 26) || false;
    sse3Supportted        = cpuinfo[2] & (1 << 0) || false;
    ssse3Supportted        = cpuinfo[2] & (1 << 9) || false;
    sse4_1Supportted    = cpuinfo[2] & (1 << 19) || false;
    sse4_2Supportted    = cpuinfo[2] & (1 << 20) || false;
    
    // ----------------------------------------------------------------------

    // Check AVX support
    // References
    // http://software.intel.com/en-us/blogs/2011/04/14/is-avx-enabled/
    // http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/
    
    avxSupportted = cpuinfo[2] & (1 << 28) || false;
    bool osxsaveSupported = cpuinfo[2] & (1 << 27) || false;
    if (osxsaveSupported && avxSupportted)
    {
        // _XCR_XFEATURE_ENABLED_MASK = 0
        unsigned long long xcrFeatureMask = _xgetbv(0);
        avxSupportted = (xcrFeatureMask & 0x6) == 0x6;
    }

    // ----------------------------------------------------------------------

    // Check SSE4a and SSE5 support

    // Get the number of valid extended IDs
    __cpuid(cpuinfo, 0x80000000);
    int numExtendedIds = cpuinfo[0];
    if (numExtendedIds >= 0x80000001)
    {
        __cpuid(cpuinfo, 0x80000001);
        sse4aSupportted = cpuinfo[2] & (1 << 6) || false;
        sse5Supportted = cpuinfo[2] & (1 << 11) || false;
    }

    // ----------------------------------------------------------------------

    std::cout << "SSE:" << (sseSupportted ? 1 : 0) << std::endl;
    std::cout << "SSE2:" << (sse2Supportted ? 1 : 0) << std::endl;
    std::cout << "SSE3:" << (sse3Supportted ? 1 : 0) << std::endl;
    std::cout << "SSE4.1:" << (sse4_1Supportted ? 1 : 0) << std::endl;
    std::cout << "SSE4.2:" << (sse4_2Supportted ? 1 : 0) << std::endl;
    std::cout << "SSE4a:" << (sse4aSupportted ? 1 : 0) << std::endl;
    std::cout << "SSE5:" << (sse5Supportted ? 1 : 0) << std::endl;
    std::cout << "AVX:" << (avxSupportted ? 1 : 0) << std::endl;

    return 0;
}*/
