CFLAGS += -Wall -Wextra -Werror
OUT ?= build
NAME ?= bidoof
EXTS ?= builtin views

ifeq ($(OS), Windows_NT)
  as-exe = $(1).exe
  as-lib = $(1).dll
  MD ?= mkdir #md
else
  as-exe = $(1)
  as-lib = lib$(1).so
  MD ?= mkdir -p
endif

all: $(OUT) $(foreach ext,$(EXTS),$(OUT)/$(call as-lib,$(ext))) $(OUT)/$(call as-exe,$(NAME))

$(OUT):
	$(MD) '$(OUT)'

$(OUT)/$(call as-exe,$(NAME)): *.[ch]
	$(CC) $^ -o '$@' -DEXTS_NAMES='$(foreach ext,$(EXTS),"$(call as-lib,$(ext))",)' $(CFLAGS)

$(OUT)/$(call as-lib,%): ext/%.c helper.h
	$(CC) $^ -o '$@' -fPIC -shared $(CFLAGS)
