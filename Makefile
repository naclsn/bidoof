CFLAGS += -Wall -Wextra -Werror
NAME ?= bidoof
EXTS ?= builtin views

ifeq ($(OS), Windows_NT)
  as-exe = $(1).exe
  as-lib = $(1).dll
else
  as-exe = $(1)
  as-lib = lib$(1).so
endif

all: $(foreach ext,$(EXTS),$(call as-lib,$(ext))) $(call as-exe,$(NAME))
	@printf '%s\n' $^

$(call as-exe,$(NAME)): *.[ch]
	$(CC) $^ -o $@ $(CFLAGS) -DEXTS_NAMES='$(foreach ext,$(EXTS),"./$(call as-lib,$(ext))",)'

$(call as-lib,%): ext/%.c
	$(CC) $^ -o $@ -I. -fPIC -shared $(CFLAGS)
