#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
ARFLAGS = -rc
CCDEP = $(CC)
CFLAGS_debug = -msse2 -g -ggdb3 -O0 -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -msse2 -O3 -ftree-vectorize -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_debug = -DDLB_GETPARAM_DOUBLE=1 -DFFTYPE_DOUBLE=1 -DKISS_FFT=1 -DWIN32=1 -D_LIB=1
DEFINES_release = -DDLB_GETPARAM_DOUBLE=1 -DFFTYPE_DOUBLE=1 -DKISS_FFT=1 -DNDEBUG=1 -DWIN32=1 -D_LIB=1
INCLUDES = -I$(BASE)dlb_clif -I$(BASE)kiss_fft130 -I$(BASE)kiss_fft130/tools -I$(BASE)src -I$(BASE)src

COMMON_FILES = $(TMP)dlb_clif/src/getparam $(TMP)src/Utilities $(TMP)src/debug $(TMP)src/dr_filters $(TMP)src/fchange $(TMP)src/fio\
               $(TMP)src/fir_filter $(TMP)src/fir_test $(TMP)src/lp_coef $(TMP)src/parse_args $(TMP)src/power $(TMP)src/sdf_writer\
               $(TMP)src/settling $(TMP)src/sos_filter $(TMP)src/ta_filters $(TMP)src/vector $(TMP)src/wavelib $(TMP)src/window

INPUTS_COMMON_ARCHIVE_debug = $(addsuffix .libsats_windows_x86_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_ARCHIVE_release = $(addsuffix .libsats_windows_x86_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_ARCHIVE_debug = $(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_debug.lib
DEP_PROJ_ARCHIVE_release = $(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_release.lib

default: libsats_debug.lib libsats_release.lib

libsats_debug.lib: $(INPUTS_COMMON_ARCHIVE_debug)
	$(AR) $(ARFLAGS)  $@ $^

libsats_release.lib: $(INPUTS_COMMON_ARCHIVE_release)
	$(AR) $(ARFLAGS)  $@ $^

$(TMP)%.libsats_windows_x86_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_debug) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) $(DEFINES_debug) $(INCLUDES) -o $@ $<

$(TMP)%.libsats_windows_x86_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .libsats_windows_x86_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .libsats_windows_x86_gnu_release.d,$(COMMON_FILES))

cleandeps:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)

clean: cleanself
	$(MAKE) -C $(BASE)kiss_fft130/libkissfft130/windows_x86_gnu cleanself

cleanself:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)
	$(RM) $(INPUTS_COMMON_ARCHIVE_debug)
	$(RM) $(INPUTS_COMMON_ARCHIVE_release)
	$(RM) libsats_debug.lib
	$(RM) libsats_release.lib

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " libsats_debug.lib"
	@echo " libsats_release.lib"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)dlb_clif/src/getparam.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/Utilities.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/debug.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/dr_filters.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/fchange.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/fio.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/fir_filter.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/fir_test.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/lp_coef.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/parse_args.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/power.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/sdf_writer.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/settling.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/sos_filter.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/ta_filters.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/vector.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/wavelib.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/window.libsats_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)dlb_clif/src/getparam.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/Utilities.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/debug.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/dr_filters.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/fchange.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/fio.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/fir_filter.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/fir_test.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/lp_coef.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/parse_args.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/power.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/sdf_writer.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/settling.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/sos_filter.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/ta_filters.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/vector.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/wavelib.libsats_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/window.libsats_windows_x86_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_debug.lib \
$(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_release.lib: %.lib: FORCE
	$(MAKE) -C $(@D) $(@F)


