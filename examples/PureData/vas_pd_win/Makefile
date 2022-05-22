# Makefile to build class 'vas_binaural' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name

INCLUDES = -I../../../source -I../vas_binaural~
INCLUDES += -I../../../pffft
INCLUDES += -I../../../C-Thread-Pool_WIN
INCLUDES += -I../../../libmysofa/src/hrtf 
INCLUDES += -I../../../libmysofa/src/hdf 
INCLUDES += -I../../../libmysofa/src/resampler
SOURCE := ../../../source
PFFFT := ../../../pffft
LIBMYSOFA := ../../../libmysofa/src
THPOOL:= ../../../C-Thread-Pool_WIN
CC=gcc

$(info $$SOURCE is [${SOURCE}])
OUTPUT_PATH = /project1/bin/$(notdir $(CURDIR))
$(info $$CURDIR is [${CURDIR}])


lib.name = vas_binaural

# input source file (class name == source file basename)
class.sources =: ../vas_partconv~.c

#vas_binaural~.class.sources = $(SOURCE)/vas_dynamicFirChannel.c 
#vas_binaural~.class.sources += $(SOURCE)/vas_fir_binaural.c $(SOURCE)/vas_fir_list.c $(SOURCE)/vas_fir.c $(SOURCE)/vas_mem.c
#vas_binaural~.class.sources += $(SOURCE)/vas_util.c $(SOURCE)/vas_fir_read.c $(SOURCE)/vas_pdmaxobject.c
#vas_binaural~.class.sources += $(PFFFT)/pffft.c $(PFFFT)/pffft_common.c 
#vas_binaural~.class.sources += $(THPOOL)/thpool.c

#vas_binaural~.class.sources += $(LIBMYSOFA)/hrtf/cache.c $(LIBMYSOFA)/hrtf/check.c $(LIBMYSOFA)/hrtf/easy.c $(LIBMYSOFA)/hrtf/interpolate.c 
#vas_binaural~.class.sources += $(LIBMYSOFA)/hrtf/kdtree.c $(LIBMYSOFA)/hrtf/lookup.c $(LIBMYSOFA)/hrtf/loudness.c $(LIBMYSOFA)/hrtf/minphase.c
#vas_binaural~.class.sources += $(LIBMYSOFA)/hrtf/neighbors.c $(LIBMYSOFA)/hrtf/reader.c $(LIBMYSOFA)/hrtf/resample.c $(LIBMYSOFA)/hrtf/spherical.c
#vas_binaural~.class.sources += $(LIBMYSOFA)/hrtf/tools.c $(LIBMYSOFA)/hdf/btree.c $(LIBMYSOFA)/hdf/dataobject.c $(LIBMYSOFA)/hdf/fractalhead.c
#vas_binaural~.class.sources += $(LIBMYSOFA)/hdf/gcol.c $(LIBMYSOFA)/hdf/gunzip.c $(LIBMYSOFA)/hdf/superblock.c $(LIBMYSOFA)/resampler/speex_resampler.c

vas_partconv~.class.sources = $(SOURCE)/vas_dynamicFirChannel.c $(SOURCE)/vas_fir_partitioned.c 
vas_partconv~.class.sources += $(SOURCE)/vas_fir_binaural.c $(SOURCE)/vas_fir_list.c $(SOURCE)/vas_fir.c $(SOURCE)/vas_mem.c
vas_partconv~.class.sources += $(SOURCE)/vas_util.c $(SOURCE)/vas_fir_read.c $(SOURCE)/vas_pdmaxobject.c $(SOURCE)/vas_thpool_noMalloc.c
vas_partconv~.class.sources += $(PFFFT)/pffft.c $(PFFFT)/pffft_common.c 


# all extra files to be included in binary distribution of the library
datafiles =

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/

CC += $(INCLUDES) -DPUREDATA -DVAS_WITH_AVERAGE_SEGMENTPOWER -DVAS_USE_MULTITHREADCONVOLUTION
#CC +=  -mavx -DVAS_USE_AVX

include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder




