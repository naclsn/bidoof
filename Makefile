warnings = -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-value
CFLAGS += -std=c99 $(warnings)

builddir = build
toolnames = archives checks compressions encodings encryptions images views

all: list

toolobjs = $(foreach t,$(toolnames),$(builddir)/t-$(t).o)

# if it exists, it should define x-toolnames and the associated $(builddir)/t-%.o targets
-include unreleased/Makefile
ifdef x-toolnames
CFLAGS += -Iunreleased -Ibidoof/tools
x-toolobjs = $(foreach t,$(x-toolnames),$(builddir)/t-$(t).o)
endif

objs = $(builddir)/bdf-base.o $(toolobjs) $(x-toolobjs)
list:; @echo $(toolnames) $(x-toolnames)
clean:; $(RM) $(objs) $(builddir)/d-*.exe
.PHONY: all list clean

ifeq ($(OS), Windows_NT)
views-LDFLAGS = -pthread -lopengl32 -lgdi32
else
views-LDFLAGS = -pthread -lGL -lX11
endif

$(builddir)/bdf-base.o: bidoof/base.h bidoof/utils/*.h; $(CC) -x c -c $< -o $@ -DBIDOOF_IMPLEMENTATION $(CFLAGS)
$(builddir)/t-%.o: bidoof/tools/%.h $(builddir)/bdf-base.o; $(CC) -x c -c $< -o $@ -DBIDOOF_T_IMPLEMENTATION $(CFLAGS) $($*-CFLAGS) $($*-LDFLAGS)
$(builddir)/d-%.exe: %.c; $(CC) $< -o $@ -DBIDOOF_LIST_DEPS $(CFLAGS) -Wl,--unresolved-symbols=ignore-all
$(builddir)/%: %.c $(builddir)/d-%.exe $(objs); $(CC) $< $(builddir)/bdf-base.o -o $@ $(CFLAGS) $(foreach t,$(shell $(builddir)/d-$*.exe),$(builddir)/t-$(t).o $($(t)-CFLAGS) $($(t)-LDFLAGS))
.PRECIOUS: $(builddir)/d-%.exe $(objs)
