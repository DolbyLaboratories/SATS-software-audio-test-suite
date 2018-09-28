#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
ARFLAGS = -rc
CCDEP = $(CC)
CFLAGS_debug = -msse2 -g -ggdb3 -O0 -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -msse2 -O3 -ftree-vectorize -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_debug = -DFFTYPE_DOUBLE=1 -DWIN32=1 -D_LIB=1
DEFINES_release = -DFFTYPE_DOUBLE=1 -DNDEBUG=1 -DWIN32=1 -D_LIB=1
INCLUDES = -I$(BASE)kiss_fft130 -I$(BASE)kiss_fft130/tools

COMMON_FILES = $(TMP)kiss_fft130/kiss_fft $(TMP)kiss_fft130/tools/kfc $(TMP)kiss_fft130/tools/kiss_fastfir $(TMP)kiss_fft130/tools/kiss_fftnd\
               $(TMP)kiss_fft130/tools/kiss_fftndr $(TMP)kiss_fft130/tools/kiss_fftr

INPUTS_COMMON_ARCHIVE_debug = $(addsuffix .libkissfft130_windows_x86_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_ARCHIVE_release = $(addsuffix .libkissfft130_windows_x86_gnu_release.o,$(COMMON_FILES))


default: libkissfft130_debug.lib libkissfft130_release.lib

libkissfft130_debug.lib: $(INPUTS_COMMON_ARCHIVE_debug)
	$(AR) $(ARFLAGS)  $@ $^

libkissfft130_release.lib: $(INPUTS_COMMON_ARCHIVE_release)
	$(AR) $(ARFLAGS)  $@ $^

$(TMP)%.libkissfft130_windows_x86_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_debug) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) $(DEFINES_debug) $(INCLUDES) -o $@ $<

$(TMP)%.libkissfft130_windows_x86_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .libkissfft130_windows_x86_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .libkissfft130_windows_x86_gnu_release.d,$(COMMON_FILES))

cleandeps:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)
	$(RM) $(INPUTS_COMMON_ARCHIVE_debug)
	$(RM) $(INPUTS_COMMON_ARCHIVE_release)
	$(RM) libkissfft130_debug.lib
	$(RM) libkissfft130_release.lib

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " libkissfft130_debug.lib"
	@echo " libkissfft130_release.lib"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)kiss_fft130/kiss_fft.libkissfft130_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)kiss_fft130/tools/kfc.libkissfft130_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fastfir.libkissfft130_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fftnd.libkissfft130_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fftndr.libkissfft130_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fftr.libkissfft130_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)kiss_fft130/kiss_fft.libkissfft130_windows_x86_gnu_release.d)
include $(wildcard $(TMP)kiss_fft130/tools/kfc.libkissfft130_windows_x86_gnu_release.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fastfir.libkissfft130_windows_x86_gnu_release.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fftnd.libkissfft130_windows_x86_gnu_release.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fftndr.libkissfft130_windows_x86_gnu_release.d)
include $(wildcard $(TMP)kiss_fft130/tools/kiss_fftr.libkissfft130_windows_x86_gnu_release.d)


