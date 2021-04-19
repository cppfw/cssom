include prorab.mk
include prorab-test.mk
include myci.mk

$(eval $(call myci-try-include, $(CONANBUILDINFO_DIR)conanbuildinfo.mak))

$(eval $(call prorab-config, ../../config))

this_name := harness

this_soname := 0

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lcssom -lpapki
this_ldflags += -L$(d)../../src/out/$(c)

this_cxxflags += $(addprefix -I,$(CONAN_INCLUDE_DIRS))
this_ldflags += $(addprefix -L,$(CONAN_LIB_DIRS))

this_no_install := true

$(eval $(prorab-build-lib))

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libcssom$(dot_so))

$(eval $(call prorab-include, ../../src/makefile))
