warnings := -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-value -Wno-unused-parameter
CFLAGS = -ggdb -O1 -std=c99 $(warnings)

build := build
bidoof := bidoof

toolnames :=      \
    archives      \
    checks        \
    compressions  \
    encodings     \
    encryptions   \
    images        \
    jvm           \
    views         \

all: list

toolobjs := $(foreach t,$(toolnames),$(build)/t-$(t).o)

# if it exists, it should define x-toolnames, `CFLAGS/LDFLAGS-$(t)`s and any rule needed outside `$(build)/t-%.o` below
-include unreleased/Makefile
ifdef x-toolnames
$(foreach t,$(x-toolnames),$(eval CFLAGS-$(t) += -Iunreleased -I$$(bidoof)/tools))
$(build)/t-%.o: unreleased/bidoof/tools/%.h bidoof/*.h; $(CC) -x c -c $< -o $@ -DBIDOOF_T_IMPLEMENTATION $(CFLAGS) $(CFLAGS-$*)
x-toolobjs := $(foreach t,$(x-toolnames),$(build)/t-$(t).o)
endif

objs = $(build)/bdf-base.o $(toolobjs) $(x-toolobjs)
list:; @echo $(toolnames) $(x-toolnames)
clean:; $(RM) $(objs) $(build)/d-*.exe
.PHONY: all list clean

ifeq ($(OS), Windows_NT)
LDFLAGS-views := -pthread -lopengl32 -lgdi32
else
LDFLAGS-views := -pthread -lGL -lX11
endif

$(build)/bdf-base.o: $(bidoof)/base.h $(bidoof)/utils/*.h; $(CC) -x c -c $< -o $@ -DBIDOOF_IMPLEMENTATION $(CFLAGS)
$(build)/t-%.o: $(bidoof)/tools/%.h $(build)/bdf-base.o; $(CC) -x c -c $< -o $@ -DBIDOOF_T_IMPLEMENTATION $(CFLAGS) $(CFLAGS-$*)
$(build)/d-%.exe: %.c; $(CC) $< -o $@ -DBIDOOF_LIST_DEPS $(CFLAGS) -Wl,--unresolved-symbols=ignore-all
$(build)/%: %.c $(build)/d-%.exe $(objs); $(CC) $< $(build)/bdf-base.o $(foreach t,$(shell $(build)/d-$*.exe),$(build)/t-$(t).o $(LDFLAGS-$(t))) -o $@
.PRECIOUS: $(build)/d-%.exe $(objs)

# --- cintre
cintre  := ../cintre/cintre
prog    := bidoof
entries := $(bidoof)/base.h $(foreach t,$(toolnames),$(bidoof)/tools/$(t).h) $(foreach t,$(x-toolnames),unreleased/bidoof/tools/$(t).h)
objs    := $(build)/bdf-base.o $(toolobjs) $(x-toolobjs)
$(foreach t,$(x-toolnames),$(eval CFLAGS-a-$(t) := -Iunreleased -I$$(bidoof)/tools))
LDFLAGS := $(foreach t,$(toolnames) $(x-toolnames),$(LDFLAGS-$(t)))
-include $(cintre)/../driver.makefile
