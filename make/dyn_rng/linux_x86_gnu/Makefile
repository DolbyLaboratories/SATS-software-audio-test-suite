#-*-makefile-*-

BASE=../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CCDEP = $(CC)
CFLAGS_debug = -msse2 -g -ggdb3 -O0 -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
CFLAGS_release = -msse2 -O3 -ftree-vectorize -m32 -c -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -pedantic -std=gnu99 -Wdeclaration-after-statement -Wvla
DEFINES_release = -DNDEBUG=1
LD = $(CC)
LDFLAGS_debug = -m32
LDFLAGS_release = -O2 -m32
LDLIBS = -lm

COMMON_FILES = $(TMP)src/dyn_rng $(TMP)src/power_vs_time

INPUTS_COMMON_LINK_debug = $(addsuffix .dyn_rng_linux_x86_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_LINK_release = $(addsuffix .dyn_rng_linux_x86_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_LINK_debug = $(BASE)make/libsats/linux_x86_gnu/libsats_debug.a $(BASE)kiss_fft130/libkissfft130/linux_x86_gnu/libkissfft130_debug.a
DEP_PROJ_LINK_release = $(BASE)make/libsats/linux_x86_gnu/libsats_release.a $(BASE)kiss_fft130/libkissfft130/linux_x86_gnu/libkissfft130_release.a

default: dyn_rng_debug dyn_rng_release

dyn_rng_debug: $(INPUTS_COMMON_LINK_debug) $(DEP_PROJ_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

dyn_rng_release: $(INPUTS_COMMON_LINK_release) $(DEP_PROJ_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.dyn_rng_linux_x86_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) -o $@ $<

$(TMP)%.dyn_rng_linux_x86_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -msse2 -MM $(DEFINES_release) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .dyn_rng_linux_x86_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .dyn_rng_linux_x86_gnu_release.d,$(COMMON_FILES))

cleandeps:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)

clean: cleanself
	$(MAKE) -C $(BASE)kiss_fft130/libkissfft130/linux_x86_gnu cleanself
	$(MAKE) -C $(BASE)make/libsats/linux_x86_gnu cleanself

cleanself:
	$(RM) $(CLEAN_COMMON_COMPILE_debug)
	$(RM) $(CLEAN_COMMON_COMPILE_release)
	$(RM) $(INPUTS_COMMON_LINK_debug)
	$(RM) $(INPUTS_COMMON_LINK_release)
	$(RM) dyn_rng_debug
	$(RM) dyn_rng_release

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " dyn_rng_debug"
	@echo " dyn_rng_release"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)src/dyn_rng.dyn_rng_linux_x86_gnu_debug.d)
include $(wildcard $(TMP)src/power_vs_time.dyn_rng_linux_x86_gnu_debug.d)
include $(wildcard $(TMP)src/dyn_rng.dyn_rng_linux_x86_gnu_release.d)
include $(wildcard $(TMP)src/power_vs_time.dyn_rng_linux_x86_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/linux_x86_gnu/libkissfft130_debug.a \
$(BASE)kiss_fft130/libkissfft130/linux_x86_gnu/libkissfft130_release.a \
$(BASE)make/libsats/linux_x86_gnu/libsats_debug.a \
$(BASE)make/libsats/linux_x86_gnu/libsats_release.a: %.a: FORCE
	$(MAKE) -C $(@D) $(@F)


