#-*-Top-level makefile-*-

la = linux_amd64_gnu
lx = linux_x86_gnu
wx = windows_x86_gnu

define make_call_common
	(cd amp_vs_time/$($(1)); $(2))
	(cd dyn_rng/$($(1)); $(2))
	(cd freq_resp/$($(1)); $(2))
	(cd noise_mod/$($(1)); $(2))
	(cd pwr_vs_time/$($(1)); $(2))
	(cd thd_vs_freq/$($(1)); $(2))
	(cd thd_vs_level/$($(1)); $(2))
endef

define make_kiss
	(cd mult_freq_resp/$($(1)); $(2))
	(cd spectrum_avg/$($(1)); $(2))
	(cd spectrum_NFFT/$($(1)); $(2))
endef

define make_intel
	(cd mult_freq_resp_intel/$($(1)); $(2))
	(cd spectrum_avg_intel/$($(1)); $(2))
	(cd spectrum_NFFT_intel/$($(1)); $(2))
endef


linux_kiss_all:
	$(call make_call_common,la,make)
	$(call make_kiss,la,make)
	$(call make_call_common,lx,make)
	$(call make_kiss,lx,make)

linux_intel_all:
	$(call make_call_common,la,make)
	$(call make_intel,la,make)
	$(call make_call_common,lx,make)
	$(call make_intel,lx,make)

linux_clean:
	$(call make_call_common,la,make clean)
	$(call make_kiss,la,make clean)
	$(call make_intel,la,make clean)
	$(call make_call_common,lx,make clean)
	$(call make_kiss,lx,make clean)
	$(call make_intel,lx,make clean)

windows_kiss_all:
	$(call make_call_common,wx,CC="gcc" mingw32-make)
	$(call make_kiss,wx,CC="gcc" mingw32-make)

windows_clean:
	$(call make_call_common,wx,mingw32-make clean)
	$(call make_kiss,wx,mingw32-make clean)
	
