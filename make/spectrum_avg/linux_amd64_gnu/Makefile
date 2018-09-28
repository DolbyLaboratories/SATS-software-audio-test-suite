#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CCDEP = $(CC)
CFLAGS_debug = -g -ggdb3 -O0 -m64 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -O3 -ftree-vectorize -m64 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_debug = -DFFTYPE_DOUBLE=1 -DKISS_FFT=1
DEFINES_release = -DFFTYPE_DOUBLE=1 -DKISS_FFT=1 -DNDEBUG=1
INCLUDES = -I$(BASE)kiss_fft130 -I$(BASE)kiss_fft130/tools -I$(BASE)src
LD = $(CC)
LDFLAGS_debug = -m64
LDFLAGS_release = -O2 -m64
LDLIBS = -lm

COMMON_FILES = $(TMP)src/fft_avg $(TMP)src/spectrum_averaging $(TMP)src/spectrum_avg

INPUTS_COMMON_LINK_debug = $(addsuffix .spectrum_avg_linux_amd64_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_LINK_release = $(addsuffix .spectrum_avg_linux_amd64_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_LINK_debug = $(BASE)make/libsats/linux_amd64_gnu/libsats_debug.a $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a
DEP_PROJ_LINK_release = $(BASE)make/libsats/linux_amd64_gnu/libsats_release.a $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a

default: spectrum_avg_debug spectrum_avg_release

spectrum_avg_debug: $(INPUTS_COMMON_LINK_debug) $(DEP_PROJ_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

spectrum_avg_release: $(INPUTS_COMMON_LINK_release) $(DEP_PROJ_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.spectrum_avg_linux_amd64_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM $(DEFINES_debug) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) $(DEFINES_debug) $(INCLUDES) -o $@ $<

$(TMP)%.spectrum_avg_linux_amd64_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .spectrum_avg_linux_amd64_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .spectrum_avg_linux_amd64_gnu_release.d,$(COMMON_FILES))

cleandeps:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)

clean: cleanself
	$(MAKE) -C $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu cleanself
	$(MAKE) -C $(BASE)make/libsats/linux_amd64_gnu cleanself

cleanself:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)
	$(RM) $(INPUTS_COMMON_LINK_debug)
	$(RM) $(INPUTS_COMMON_LINK_release)
	$(RM) spectrum_avg_debug
	$(RM) spectrum_avg_release

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " spectrum_avg_debug"
	@echo " spectrum_avg_release"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)src/fft_avg.spectrum_avg_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/spectrum_averaging.spectrum_avg_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/spectrum_avg.spectrum_avg_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/fft_avg.spectrum_avg_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/spectrum_averaging.spectrum_avg_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/spectrum_avg.spectrum_avg_linux_amd64_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a \
$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a \
$(BASE)make/libsats/linux_amd64_gnu/libsats_debug.a \
$(BASE)make/libsats/linux_amd64_gnu/libsats_release.a: %.a: FORCE
	$(MAKE) -C $(@D) $(@F)


