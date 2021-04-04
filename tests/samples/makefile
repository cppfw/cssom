include prorab.mk

$(eval $(call prorab-config, ../../config))

this_name := parsecss

this_srcs += main.cpp
this_srcs += ../harness/properties.cpp

this_ldlibs += -lcssom -lpapki -lclargs
this_ldflags += -L$(d)../../src/out/$(c)

this_no_install := true

$(eval $(prorab-build-app))

this_samples := $(shell ls $(d)testdata/*.css)

define this_rules
test:: $(prorab_this_name)
$(.RECIPEPREFIX)@myci-running-test.sh $(notdir $(abspath $(d)))
$(.RECIPEPREFIX)$(a)cp $(d)../../src/out/$(c)/*.dll $(d)out/$(c) || true
$(.RECIPEPREFIX)$(a)for f in $(this_samples); do \
	    echo "checking file $$$$f" && \
	    LD_LIBRARY_PATH=$(d)../../src/out/$(c) DYLD_LIBRARY_PATH=$$$$LD_LIBRARY_PATH $(d)./out/$(c)/parsecss --out-file=$(d)out.css $$$$f && \
	    cmp $(d)out.css $$$$f.cmp; \
	    if [ $$$$? -ne 0 ]; then myci-error.sh "parsed CSS did not match etalon sample"; exit 1; fi \
	done
$(.RECIPEPREFIX)@myci-passed.sh
endef
$(eval $(this_rules))

# add dependency on libcssom
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libcssom$(dot_so))

$(eval $(call prorab-include, ../../src/makefile))
