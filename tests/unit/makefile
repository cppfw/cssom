include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -l papki$(this_dbg)
this_ldlibs += -l utki$(this_dbg)
this_ldlibs += -l tst

this_ldlibs += ../../src/out/$(c)/libcssom$(this_dbg)$(dot_so)
this_ldlibs += ../harness/out/$(c)/libharness$(this_dbg)$(dot_so)

this_cxxflags += -isystem ../../src

this_no_install := true

$(eval $(prorab-build-app))

this_test_cmd := $(prorab_this_name) --junit-out=out/$(c)/junit.xml
this_test_deps := $(prorab_this_name)
this_test_ld_path := ../../src/out/$(c) ../harness/out/$(c)
$(eval $(prorab-test))

$(eval $(call prorab-include, ../../src/makefile))
$(eval $(call prorab-include, ../harness/makefile))
