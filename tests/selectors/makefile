include prorab.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lharness -lcssom -lpapki
this_ldflags += -L$(d)../../src/out/$(c)
this_ldflags += -L$(d)../harness/out/$(c)

this_no_install := true

$(eval $(prorab-build-app))

include $(d)../test_target.mk

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libcssom$(dot_so))

# add dependency on libharness
$(prorab_this_name): $(abspath $(d)../harness/out/$(c)/libharness$(dot_so))

$(eval $(call prorab-include, ../../src/makefile))
$(eval $(call prorab-include, ../harness/makefile))
