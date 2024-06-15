warnings := -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-value -Wno-unused-parameter
CFLAGS = -ggdb -O1 -std=c99 $(warnings)

build := build
bidoof := bidoof

toolnames := archives checks compressions encodings encryptions images ${jvm -> TODO: bitfields =} views

all: list

toolobjs := $(foreach t,$(toolnames),$(build)/t-$(t).o)

# if it exists, it should define x-toolnames and the associated $(build)/t-%.o targets
-include unreleased/Makefile
ifdef x-toolnames
CFLAGS += -Iunreleased -I$(bidoof)/tools
x-toolobjs := $(foreach t,$(x-toolnames),$(build)/t-$(t).o)
endif

objs = $(build)/bdf-base.o $(toolobjs) $(x-toolobjs)
list:; @echo $(toolnames) $(x-toolnames)
clean:; $(RM) $(objs) $(build)/d-*.exe
.PHONY: all list clean

ifeq ($(OS), Windows_NT)
views-LDFLAGS := -pthread -lopengl32 -lgdi32
else
views-LDFLAGS := -pthread -lGL -lX11
endif

$(build)/bdf-base.o: $(bidoof)/base.h $(bidoof)/utils/*.h; $(CC) -x c -c $< -o $@ -DBIDOOF_IMPLEMENTATION $(CFLAGS)
$(build)/t-%.o: $(bidoof)/tools/%.h $(build)/bdf-base.o; $(CC) -x c -c $< -o $@ -DBIDOOF_T_IMPLEMENTATION $(CFLAGS) $($*-CFLAGS) $($*-LDFLAGS)
$(build)/d-%.exe: %.c; $(CC) $< -o $@ -DBIDOOF_LIST_DEPS $(CFLAGS) -Wl,--unresolved-symbols=ignore-all
$(build)/%: %.c $(build)/d-%.exe $(objs); $(CC) $< $(build)/bdf-base.o -o $@ $(CFLAGS) $(foreach t,$(shell $(build)/d-$*.exe),$(build)/t-$(t).o $($(t)-CFLAGS) $($(t)-LDFLAGS))
.PRECIOUS: $(build)/d-%.exe $(objs)

# --- cintre
cintre  := ../cintre/cintre
prog    := bidoof
entries := $(bidoof)/base.h $(foreach t,$(toolnames),$(bidoof)/tools/$(t).h) $(foreach t,$(x-toolnames),unreleased/bidoof/tools/$(t).h)
objs    := $(build)/bdf-base.o $(toolobjs) $(x-toolobjs)
LDFLAGS := $(foreach t,$(toolnames) $(x-toolnames),$($(t)-LDFLAGS))
-include $(cintre)/../driver.makefile
