#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CCDEP = $(CC)
CFLAGS_debug = -g -ggdb3 -O0 -m64 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -O3 -ftree-vectorize -m64 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_release = -DNDEBUG=1
LD = $(CC)
LDFLAGS_debug = -m64
LDFLAGS_release = -O2 -m64
LDLIBS = -lm

COMMON_FILES = $(TMP)src/freq_resp $(TMP)src/frequency_response $(TMP)src/thd_freq

INPUTS_COMMON_LINK_debug = $(addsuffix .freq_resp_linux_amd64_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_LINK_release = $(addsuffix .freq_resp_linux_amd64_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_LINK_debug = $(BASE)make/libsats/linux_amd64_gnu/libsats_debug.a $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a
DEP_PROJ_LINK_release = $(BASE)make/libsats/linux_amd64_gnu/libsats_release.a $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a

default: freq_resp_debug freq_resp_release

freq_resp_debug: $(INPUTS_COMMON_LINK_debug) $(DEP_PROJ_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

freq_resp_release: $(INPUTS_COMMON_LINK_release) $(DEP_PROJ_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.freq_resp_linux_amd64_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) -o $@ $<

$(TMP)%.freq_resp_linux_amd64_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM $(DEFINES_release) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .freq_resp_linux_amd64_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .freq_resp_linux_amd64_gnu_release.d,$(COMMON_FILES))

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
	$(RM) freq_resp_debug
	$(RM) freq_resp_release

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " freq_resp_debug"
	@echo " freq_resp_release"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)src/freq_resp.freq_resp_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/frequency_response.freq_resp_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/thd_freq.freq_resp_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/freq_resp.freq_resp_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/frequency_response.freq_resp_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/thd_freq.freq_resp_linux_amd64_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a \
$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a \
$(BASE)make/libsats/linux_amd64_gnu/libsats_debug.a \
$(BASE)make/libsats/linux_amd64_gnu/libsats_release.a: %.a: FORCE
	$(MAKE) -C $(@D) $(@F)


