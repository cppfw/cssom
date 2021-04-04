include prorab.mk

$(eval $(call prorab-config, ../../config))

this_name := harness

this_soname := 0

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lcssom -lpapki
this_ldflags += -L$(d)../../src/out/$(c)

this_no_install := true

$(eval $(prorab-build-lib))

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libcssom$(dot_so))

$(eval $(call prorab-include, ../../src/makefile))
