#include <stdio.h>
#include <stdlib.h>

#include "asrtest.h"

bool odd(int c) {
    if (c & 1) {
        ASSERT_REACH_AND(ar_odd_1, ((c-1) & 1) == 0);
#define ar_odd_1 odd(1)
        return true;
    }

    else {
        ASSERT_REACH_AND(ar_odd_2, c*c == c+c);
#define ar_odd_2 odd(2)
        return false;
    }

    ASSERT_NOREACH(ar_odd_unreachable);
#define ar_odd_unreachable odd(42)
}

void bla(void) {
    ASSERT_REACH(ar_bla);
#define ar_bla bla();
}

ASR_MAIN {
    ASR_TEST(ar_odd_1);
    ASR_TEST(ar_odd_2);
    ASR_TEST(ar_odd_unreachable);
    ASR_TEST(ar_bla);
    ASR_MAIN_RETURN;
}
