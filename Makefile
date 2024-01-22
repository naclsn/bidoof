CFLAGS += -ggdb -std=c99 -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -Wno-unused-variable #-Wfatal-errors

builddir = build
toolnames = archives encodings images

toolobjs = $(foreach t,$(toolnames),$(builddir)/t-$(t).o)
all: $(toolobjs)

$(builddir)/t-%.o: bidoof/tools/%.h bidoof/*.h; $(CC) -x c -c $< -o $@ $(CFLAGS) -DBDF_IMPLEMENTATION
$(builddir)/%: %.c all; $(CC) $< $(toolobjs) -o $@ $(CFLAGS)
