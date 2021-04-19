include prorab.mk
include prorab-test.mk
include myci.mk

$(eval $(call myci-try-include, $(CONANBUILDINFO_DIR)conanbuildinfo.mak))

$(eval $(call prorab-config, ../../config))

this_name := parsecss

this_srcs += main.cpp
this_srcs += ../harness/properties.cpp

this_ldlibs += -lcssom -lpapki -lclargs
this_ldflags += -L$(d)../../src/out/$(c)

this_cxxflags += $(addprefix -I,$(CONAN_INCLUDE_DIRS))
this_ldflags += $(addprefix -L,$(CONAN_LIB_DIRS))

this_no_install := true

$(eval $(prorab-build-app))

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libcssom$(dot_so))

this_test_cmd := for f in $(shell ls $(d)testdata/*.css); do \
	    echo "checking file $$$$f" && \
	    $(prorab_this_name) --out-file=$(d)out.css $$$$f && \
	    cmp $(d)out.css $$$$f.cmp; \
	    if [ $$$$? -ne 0 ]; then myci-error.sh "parsed CSS did not match etalon sample"; exit 1; fi \
	done
this_test_deps := $(prorab_this_name)
this_test_ld_path := ../../src/out/$(c) $(CONAN_LIB_DIRS)
$(eval $(prorab-test))

$(eval $(call prorab-include, ../../src/makefile))
