include prorab.mk

this_name := tests

this_out_dir := build

this_cxxflags += -Wall
this_cxxflags += -Werror
this_cxxflags += -funsigned-char # the 'char' type is unsigned
this_cxxflags += -fstrict-aliasing
this_cxxflags += -std=c++14
this_cxxflags += -g

ifeq ($(debug),true)
	this_cxxflags += -DDEBUG
else
	this_cxxflags += -O3
endif

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lharness -lcssom -lpapki -lstdc++
this_ldflags += -L$(d)../../src/build
this_ldflags += -L$(d)../harness/build

ifeq ($(os), linux)
    this_cxxflags += -fPIC
    this_ldlibs +=
else ifeq ($(os), macosx)
    this_cxxflags += -stdlib=libc++ # this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
else ifeq ($(os),windows)
endif

this_no_install := true

$(eval $(prorab-build-app))

include $(d)../test_target.mk

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/build/libcssom$(dot_so))

# add dependency on libharness
$(prorab_this_name): $(abspath $(d)../harness/build/libharness$(dot_so))

$(eval $(call prorab-include, ../../src/makefile))
$(eval $(call prorab-include, ../harness/makefile))
