include prorab.mk
include prorab-license.mk
include prorab-clang-format.mk
include prorab-install-dbgsrc.mk

$(eval $(call prorab-config, ../config))

this_name := cssom

this_soname := $(shell cat $(d)soname.txt)

this_srcs := $(call prorab-src-dir,.)

this_ldlibs += -lpapki -lutki

$(eval $(prorab-build-lib))

$(eval $(prorab-clang-format))

this_src_dir := $(this_name)
this_license_file := ../LICENSE
$(eval $(prorab-license))

$(eval $(prorab-install-dbgsrc))
