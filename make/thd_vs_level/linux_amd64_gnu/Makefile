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

COMMON_FILES = $(TMP)src/thd_ampl $(TMP)src/thd_vs_level

INPUTS_COMMON_LINK_debug = $(addsuffix .thd_vs_level_linux_amd64_gnu_debug.o,$(COMMON_FILES))
INPUTS_COMMON_LINK_release = $(addsuffix .thd_vs_level_linux_amd64_gnu_release.o,$(COMMON_FILES))

DEP_PROJ_LINK_debug = $(BASE)make/libsats/linux_amd64_gnu/libsats_debug.a $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a
DEP_PROJ_LINK_release = $(BASE)make/libsats/linux_amd64_gnu/libsats_release.a $(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a

default: thd_vs_level_debug thd_vs_level_release

thd_vs_level_debug: $(INPUTS_COMMON_LINK_debug) $(DEP_PROJ_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

thd_vs_level_release: $(INPUTS_COMMON_LINK_release) $(DEP_PROJ_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.thd_vs_level_linux_amd64_gnu_debug.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_debug) -o $@ $<

$(TMP)%.thd_vs_level_linux_amd64_gnu_release.o: $(BASE)%.c | $(TMP)%/..
	$(CCDEP) -MM $(DEFINES_release) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS_release) $(DEFINES_release) -o $@ $<

CLEAN_COMMON_COMPILE_debug = $(addsuffix .thd_vs_level_linux_amd64_gnu_debug.d,$(COMMON_FILES))
CLEAN_COMMON_COMPILE_release = $(addsuffix .thd_vs_level_linux_amd64_gnu_release.d,$(COMMON_FILES))

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
	$(RM) thd_vs_level_debug
	$(RM) thd_vs_level_release

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " thd_vs_level_debug"
	@echo " thd_vs_level_release"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)src/thd_ampl.thd_vs_level_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/thd_vs_level.thd_vs_level_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)src/thd_ampl.thd_vs_level_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)src/thd_vs_level.thd_vs_level_linux_amd64_gnu_release.d)

$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_debug.a \
$(BASE)kiss_fft130/libkissfft130/linux_amd64_gnu/libkissfft130_release.a \
$(BASE)make/libsats/linux_amd64_gnu/libsats_debug.a \
$(BASE)make/libsats/linux_amd64_gnu/libsats_release.a: %.a: FORCE
	$(MAKE) -C $(@D) $(@F)


