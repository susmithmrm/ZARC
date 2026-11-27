#include "zarc/domain.h"
#include <stdio.h>
int main(void) {
    zarc_domain domain;
    zarc_effect effect;
    zarc_init(&domain);
    (void)zarc_feedback(&domain, ZARC_OFF, 0);
    if (zarc_request(&domain, true, 1, ZARC_ON, 1, 10, &effect) != ZARC_ACCEPTED || !effect.emitted) return 1;
    if (domain.observed != ZARC_OFF) return 1;
    if (zarc_feedback(&domain, effect.desired, 2) != ZARC_COMPLETED) return 1;
    puts("Simulated indicator: accepted, independently observed, completed.");
    return 0;
}
