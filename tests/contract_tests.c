#include "zarc/domain.h"
#include <stdio.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s failed\n", __LINE__, #expr); return 1; } } while (0)
int main(void) {
    zarc_domain d, other;
    zarc_effect e;
    zarc_init(&d); zarc_init(&other);
    CHECK(d.observed == ZARC_UNKNOWN);
    (void)zarc_feedback(&d, ZARC_OFF, 0);
    CHECK(zarc_request(&d, false, 7, ZARC_ON, 1, 10, &e) == ZARC_DENIED && !e.emitted);
    CHECK(zarc_request(&d, true, 7, ZARC_ON, 1, 10, &e) == ZARC_ACCEPTED && e.emitted);
    CHECK(d.observed == ZARC_OFF);
    CHECK(zarc_request(&d, true, 7, ZARC_ON, 2, 10, &e) == ZARC_ACCEPTED && !e.emitted);
    CHECK(zarc_request(&d, true, 7, ZARC_OFF, 2, 10, &e) == ZARC_CONFLICT);
    CHECK(zarc_request(&d, true, 8, ZARC_OFF, 2, 10, &e) == ZARC_BUSY);
    CHECK(zarc_feedback(&d, ZARC_ON, 3) == ZARC_COMPLETED);
    CHECK(other.observed == ZARC_UNKNOWN);
    CHECK(zarc_request(&d, true, 8, ZARC_OFF, 4, 10, &e) == ZARC_ACCEPTED);
    CHECK(zarc_tick(&d, 10) == ZARC_UNKNOWN_OUTCOME && d.observed == ZARC_UNKNOWN);
    CHECK(zarc_feedback(&d, ZARC_OFF, 11) == ZARC_UNKNOWN_OUTCOME);
    CHECK(zarc_tick(&d, 9) == ZARC_INVALID);
    puts("Domain contract tests passed");
    return 0;
}
