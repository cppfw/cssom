include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := harness$(this_dbg)

this_soname := 0

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -l fsif$(this_dbg)
this_ldlibs += ../../src/out/$(c)/libcssom$(this_dbg)$(dot_so)

this_no_install := true

$(eval $(prorab-build-lib))

$(eval $(call prorab-include, ../../src/makefile))
