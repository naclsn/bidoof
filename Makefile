CFLAGS += -Wall -Wextra -Werror -std=c99 #-Wfatal-errors
OUT ?= build
NAME ?= bidoof
EXTS ?= archives builtin encodings views

ifeq ($(OS), Windows_NT)
  as-exe = $(1).exe
  as-lib = $(1).dll
  MD ?= mkdir -p #md
  views-LDFLAGS = -pthread -lopengl32 -lgdi32
else
  as-exe = $(1)
  as-lib = lib$(1).so
  MD ?= mkdir -p
  views-LDFLAGS = -pthread -lGL -lX11
endif
as-test = $(call as-exe,$(OUT)/test-$(1))

all: $(OUT) $(foreach ext,$(EXTS),$(OUT)/$(call as-lib,$(ext))) $(OUT)/$(call as-exe,$(NAME))
test: $(call as-test,lang)

$(OUT):
	$(MD) '$(OUT)'

$(OUT)/$(call as-exe,$(NAME)): src/*.[ch]
	$(CC) $^ -o '$@' -DEXTS_NAMES='$(foreach ext,$(EXTS),"$(call as-lib,$(ext))",)' $(CFLAGS)

$(OUT)/$(call as-lib,%): ext/%.c src/helper.h src/base.[ch]
	$(CC) $^ -o '$@' -fPIC -shared $(CFLAGS) $($*-CFLAGS) $($*-LDFLAGS)

$(OUT)/$(call as-exe,test-%): src/%.c
	$(CC) $^ -o '$@' -DASR_TEST_BUILD
	'$@'

wip/%: wip/%.c
	$(MD) '$(OUT)/wip'
	$(CC) $^ -o '$(OUT)/$@' $(CFLAGS) $(LDFLAGS)
	'$(OUT)/$@' $(ARGS)
