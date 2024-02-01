CFLAGS += -ggdb -std=c99 -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-value #-Wfatal-errors

builddir = build
toolnames = archives checks compressions encodings encryptions images

all: base

toolobjs = $(foreach t,$(toolnames),$(builddir)/t-$(t).o)

# if it exists, it should define
# x-toolnames
# x-toolobjs
-include unreleased/Makefile
ifdef x-toolnames
CFLAGS += -Iunreleased -Ibidoof/tools
endif

base: $(builddir)/bdf-base.o $(toolobjs) $(x-toolobjs)
list:; @echo $(toolnames) $(x-toolnames)

$(builddir)/bdf-base.o: bidoof/base.h bidoof/*.h; $(CC) -x c -c $< -o $@ -DBDF_BASE_IMPLEMENTATION $(CFLAGS)
$(builddir)/t-%.o: bidoof/tools/%.h bidoof/*.h; $(CC) -x c -c $< -o $@ -DBDF_IMPLEMENTATION $(CFLAGS)
$(builddir)/%: %.c all; $(CC) $< $(builddir)/bdf-base.o $(toolobjs) $(x-toolobjs) -o $@ $(CFLAGS)
