CFLAGS += -ggdb -std=c99 -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -Wno-unused-variable #-Wfatal-errors

builddir = build
toolnames = archives crcs encodings encryptions images

toolobjs = $(foreach t,$(toolnames),$(builddir)/t-$(t).o)
all: $(builddir)/bdf-base.o $(toolobjs)

$(builddir)/bdf-base.o: bidoof/base.h bidoof/*.h; $(CC) -x c -c $< -o $@ -DBDF_BASE_IMPLEMENTATION $(CFLAGS)
$(builddir)/t-%.o: bidoof/tools/%.h bidoof/*.h; $(CC) -x c -c $< -o $@ -DBDF_IMPLEMENTATION $(CFLAGS)
$(builddir)/%: %.c all; $(CC) $< $(builddir)/bdf-base.o $(toolobjs) -o $@ $(CFLAGS)
