CFLAGS += -Wall -Wextra -Werror -std=c99
OUT ?= build
NAME ?= bidoof
EXTS ?= builtin views

ifeq ($(OS), Windows_NT)
  as-exe = $(1).exe
  as-lib = $(1).dll
  MD ?= mkdir -p #md
  views-LDFLAGS = -lopengl32 -lgdi32
else
  as-exe = $(1)
  as-lib = lib$(1).so
  MD ?= mkdir -p
  views-LDFLAGS = -lGL -lX11
endif
as-test = $(call as-exe,$(OUT)/test-$(1))

all: $(OUT) $(foreach ext,$(EXTS),$(OUT)/$(call as-lib,$(ext))) $(OUT)/$(call as-exe,$(NAME))
test: $(call as-test,lang)

$(OUT):
	$(MD) '$(OUT)'

$(OUT)/$(call as-exe,$(NAME)): *.[ch]
	$(CC) $^ -o '$@' -DEXTS_NAMES='$(foreach ext,$(EXTS),"$(call as-lib,$(ext))",)' $(CFLAGS)

$(OUT)/$(call as-lib,%): ext/%.c helper.h base.[ch]
	$(CC) $^ -o '$@' -fPIC -shared $(CFLAGS) $($*-CFLAGS) $($*-LDFLAGS)

$(OUT)/$(call as-exe,test-%): %.c
	$(CC) $^ -o '$@' -DASR_TEST_BUILD
	'$@'

wip/%: wip/%.c
	$(MD) '$(OUT)/wip'
	$(CC) $^ -o '$(OUT)/$@' $(CFLAGS) $(LDFLAGS)
	'$(OUT)/$@' $(ARGS)
