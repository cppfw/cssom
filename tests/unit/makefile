include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lharness -lcssom -lpapki -lutki -ltst
this_ldflags += -L$(d)../../src/out/$(c)
this_ldflags += -L$(d)../harness/out/$(c)

this_cxxflags += -isystem ../../src

this_no_install := true

$(eval $(prorab-build-app))

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libcssom$(dot_so))

# add dependency on libharness
$(prorab_this_name): $(abspath $(d)../harness/out/$(c)/libharness$(dot_so))

this_test_cmd := $(prorab_this_name) --junit-out=out/$(c)/junit.xml
this_test_deps := $(prorab_this_name)
this_test_ld_path := ../../src/out/$(c) ../harness/out/$(c)
$(eval $(prorab-test))

$(eval $(call prorab-include, ../../src/makefile))
$(eval $(call prorab-include, ../harness/makefile))
