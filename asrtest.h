#ifndef ASR_TEST_BUILD
# define ASSERT_REACH_AND(__tag, __cdt)
# define ASSERT_REACH(__tag)
# define ASSERT_NOREACH(__tag)
# define ASR_TEST(__tag)
# define ASR_MAIN_BEGIN(__argc, __argv)
# define ASR_MAIN_END
#else // (till end of file)

#include <stdbool.h>
#include <string.h>

#ifndef asr_printf
# include <stdio.h>
# define asr_printf printf
#endif

#ifndef asr_filter
# include <fnmatch.h>
# define asr_filter(__pattern, __string) fnmatch(__pattern, __string, 0)
#endif

static char const* _asr_current;
static bool _asr_reached;

static int _asr_fails = 0;
static int _asr_misses = 0;

static bool _asr_summary = true;
static bool _asr_listing = false;
static char const* _asr_filter = NULL;

static bool _asr_run(char const* tag) { return !_asr_listing && (!_asr_filter || 0 == asr_filter(_asr_filter, tag)); }
static bool _asr_list(char const* tag) { return _asr_listing && (!_asr_filter || 0 == asr_filter(_asr_filter, tag)); }

static inline bool _asr_parseargs(int argc, char** argv) {
    argc--;
    char const* prog = *argv++;

#define if_arg(__lit) if (0 == strcmp(__lit, *argv))
    for (; *argv; ++argv) {
        if_arg ("--help") {
            asr_printf("Usage: %s ...\n"
                       "     --nosum      do not display a summary once done\n"
                       "     --list       list tests instead of running them\n"
                       "     --filter     use a glob to filter tests to use\n"
                       "\n", prog);
            return false;
        }
        else if_arg ("--nosum") {
            _asr_summary = false;
        }
        else if_arg ("--list") {
            _asr_listing = true;
            _asr_summary = false;
        }
        else if_arg ("--filter") {
            _asr_filter = *++argv;
            if (!*argv) {
                asr_printf("Expected value after filter argument\n");
                return false;
            }
        }
        else {
            asr_printf("Unknown argument: '%s'\n", *argv);
            return false;
        }
    }
#undef if_arg

    return true;
}

#define ASSERT_REACH_AND(__tag, __cdt) do                             \
    if (0 == strcmp(#__tag, _asr_current)) {                          \
        _asr_reached = true;                                          \
        asr_printf(#__tag "(" __FILE__ ":%d)\t%s\n", __LINE__, __cdt  \
            ? "pass"                                                  \
            : (_asr_fails++, "condition not verified: " #__cdt));     \
    } while (0)
#define ASSERT_REACH(__tag) ASSERT_REACH_AND(__tag, 1)
#define ASSERT_NOREACH(__tag) ASSERT_REACH_AND(__tag, !"should not reach")

#define ASR_TEST(__tag) do                                         \
    if (_asr_run(#__tag)) {                                        \
        _asr_current = #__tag;                                     \
        _asr_reached = false;                                      \
        { (void)__tag; }                                           \
        if (!_asr_reached) {                                       \
            _asr_misses++;                                         \
            asr_printf(#__tag "(" __FILE__ ":?)\tnot reached\n");  \
        }                                                          \
    } else if (_asr_list(#__tag)) {                                \
        asr_printf(#__tag "\n");                                   \
    } while (0)

#define ASR_MAIN                                              \
    int main(int argc, char** argv) {                         \
        if (!_asr_parseargs(argc, argv)) return 1;            \
        do
#define ASR_MAIN_RETURN                                       \
        } while (0);                                          \
        if (_asr_summary)                                     \
            asr_printf("\n--------\n%d fail%s\n%d miss%s\n",  \
                _asr_fails, 1 < _asr_fails ? "s" : "",        \
                _asr_misses, 1 < _asr_misses ? "es" : "");    \
        return 0;                                             \

#endif // ASR_TEST_BUILD
