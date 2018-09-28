#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
ARFLAGS = -rc
CCDEP = $(CC)
CFLAGS_debug = -g -ggdb3 -O0 -m64 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -O3 -ftree-vectorize -m64 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_debug = -DDLB_GETPARAM_DOUBLE=1 -DFFTYPE_DOUBLE=1 -DKISS_FFT=1
DEFINES_release = -DDLB_GETPARAM_DOUBLE=1 -DFFTYPE_DOUBLE=1 -DKISS_FFT=1 -DNDEBUG=1
INCLUDES = -I$(BASE)dlb_clif -I$(BASE)kiss_fft130 -I$(BASE)kiss_fft130/tools -I$(BASE)src -I$(BASE)src

COMMON_FILES = $(TMP)dlb_clif/src/getparam $(TMP)src/Utilities $(TMP)src/debug $(TMP)src/dr_filters $(TMP)src/fchange $(TMP)src/fio\
               $(TMP)src/fir_filter $(TMP)src/fir_test $(TMP)src/lp_coef $(TMP)src/parse_args $(TMP)src/power $(TMP)src/sdf_writer\
               $(TMP)src/settling $(TMP)src/sos_filter $(TMP)src/ta_filters $(TMP)src/vector $(TMP)src/wavelib $(TMP)src/window

INPUTS_COMMON_ARCHIVE_debug = $(addsuffix .libsats_linux_amd64_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_ARCHIVE_release = $(addsuffix .libsats_linux_amd64_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_ARCHIVE_debug = $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a
DEP_PROJ_ARCHIVE_release = $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a

default: libsats_debug.a libsats_release.a

libsats_debug.a: $(INPUTS_COMMON_ARCHIVE_debug)
	$(AR) $(ARFLAGS)  $@ $^

libsats_release.a: $(INPUTS_COMMON_ARCHIVE_release)
	$(AR) $(ARFLAGS)  $@ $^

$(TMP)%.libsats_linux_amd64_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM $(DEFINES_debug) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) $(DEFINES_debug) $(INCLUDES) -o $@ $<

$(TMP)%.libsats_linux_amd64_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .libsats_linux_amd64_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .libsats_linux_amd64_gnu_release.d,$(COMMON_FILES))

cleandeps:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)

clean: cleanself
	$(MAKE) -C $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu cleanself

cleanself:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)
	$(RM) $(INPUTS_COMMON_ARCHIVE_debug)
	$(RM) $(INPUTS_COMMON_ARCHIVE_release)
	$(RM) libsats_debug.a
	$(RM) libsats_release.a

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " libsats_debug.a"
	@echo " libsats_release.a"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)dlb_clif/src/getparam.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/Utilities.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/debug.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/dr_filters.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/fchange.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/fio.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/fir_filter.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/fir_test.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/lp_coef.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/parse_args.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/power.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/sdf_writer.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/settling.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/sos_filter.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/ta_filters.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/vector.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/wavelib.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/window.libsats_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)dlb_clif/src/getparam.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/Utilities.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/debug.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/dr_filters.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/fchange.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/fio.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/fir_filter.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/fir_test.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/lp_coef.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/parse_args.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/power.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/sdf_writer.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/settling.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/sos_filter.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/ta_filters.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/vector.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/wavelib.libsats_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/window.libsats_linux_amd64_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a \
$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a: %.a: FORCE
	$(MAKE) -C $(@D) $(@F)


