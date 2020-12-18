# Makefile to build class 'vas_dynconv' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name

INCLUDES = -I../../../../source -I../../../../kissfft -I../../../../kissfft/tools -I../../vas_dynconv~
KISSFFT = ../../../../kissfft
KISSFFTTOOLS = ../../../../kissfft/tools
SOURCE = ../../../../source

lib.name = vas_dynconv

# input source file (class name == source file basename)
class.sources = ../../vas_dynconv~.c

vas_dynconv~.class.sources  = $(KISSFFT)/kiss_fft.c $(KISSFFTTOOLS)/kiss_fftr.c $(SOURCE)/vas_firobject.c $(SOURCE)/vas_dynamicFirChannel.c 
vas_dynconv~.class.sources += $(SOURCE)/vas_fir_binaural.c $(SOURCE)/vas_fir_list.c $(SOURCE)/vas_fir.c $(SOURCE)/vas_mem.c
vas_dynconv~.class.sources += $(SOURCE)/vas_util_vectorIntrinsics.c $(SOURCE)/vas_util.c 

# all extra files to be included in binary distribution of the library
datafiles =

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=../pd-lib-builder/

CC += $(INCLUDES) -DPUREDATA -DVAS_WITH_AVERAGE_SEGMENTPOWER -DVAS_USE_KISSFFT
# CC +=  -mavx -DVAS_USE_AVX

include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder




