#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CCDEP = $(CC)
CFLAGS_debug = -msse2 -g -ggdb3 -O0 -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -msse2 -O3 -ftree-vectorize -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_debug = -DWIN32=1 -D_CONSOLE=1
DEFINES_release = -DNDEBUG=1 -DWIN32=1 -D_CONSOLE=1
LD = $(CC)
LDFLAGS_debug = -m32
LDFLAGS_release = -O2 -m32

COMMON_FILES = $(TMP)src/freq_resp $(TMP)src/frequency_response $(TMP)src/thd_freq

INPUTS_COMMON_LINK_debug = $(addsuffix .freq_resp_windows_x86_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_LINK_release = $(addsuffix .freq_resp_windows_x86_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_LINK_debug = $(BASE)make/libsats/windows_x86_gnu/libsats_debug.lib $(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_debug.lib
DEP_PROJ_LINK_release = $(BASE)make/libsats/windows_x86_gnu/libsats_release.lib $(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_release.lib

default: freq_resp_debug.exe freq_resp_release.exe

freq_resp_debug.exe: $(INPUTS_COMMON_LINK_debug) $(DEP_PROJ_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ 

freq_resp_release.exe: $(INPUTS_COMMON_LINK_release) $(DEP_PROJ_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ 

$(TMP)%.freq_resp_windows_x86_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_debug) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) $(DEFINES_debug) -o $@ $<

$(TMP)%.freq_resp_windows_x86_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_release) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .freq_resp_windows_x86_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .freq_resp_windows_x86_gnu_release.d,$(COMMON_FILES))

cleandeps:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)

clean: cleanself
	$(MAKE) -C $(BASE)kiss_fft130/libkissfft130/windows_x86_gnu cleanself
	$(MAKE) -C $(BASE)make/libsats/windows_x86_gnu cleanself

cleanself:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)
	$(RM) $(INPUTS_COMMON_LINK_debug)
	$(RM) $(INPUTS_COMMON_LINK_release)
	$(RM) freq_resp_debug.exe
	$(RM) freq_resp_release.exe

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " freq_resp_debug.exe"
	@echo " freq_resp_release.exe"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)src/freq_resp.freq_resp_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/frequency_response.freq_resp_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/thd_freq.freq_resp_windows_x86_gnu_debug.d)
include $(wildcard $(TMP)src/freq_resp.freq_resp_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/frequency_response.freq_resp_windows_x86_gnu_release.d)
include $(wildcard $(TMP)src/thd_freq.freq_resp_windows_x86_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_debug.lib \
$(BASE)kiss_fft130/libkissfft130/windows_x86_gnu/libkissfft130_release.lib \
$(BASE)make/libsats/windows_x86_gnu/libsats_debug.lib \
$(BASE)make/libsats/windows_x86_gnu/libsats_release.lib: %.lib: FORCE
	$(MAKE) -C $(@D) $(@F)


