CFLAGS += -Wall -Wextra -Werror

ifeq ($(OS), Windows_NT)
  as-exe = $(1).exe
  as-lib = $(1).dll
else
  as-exe = $(1)
  as-lib = lib$(1).so
endif

name = bidoof
srcs = main.c base.c loader.c
exts = builtin

$(call as-exe,$(name)): $(srcs) $(foreach ext,$(exts),$(call as-lib,$(ext)))
	$(CC) $(srcs) -o $@ $(CFLAGS)

$(call as-lib,%): ext/%.c
	$(CC) $^ -o $@ -I. -fPIC -shared $(CFLAGS)

install: $(call as-exe,$(name)) $(foreach ext,$(exts),$(call as-lib,$(ext)))
	@printf '%s\n' $^
