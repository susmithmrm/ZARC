#include "zarc/sensor.h"
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(void)
{
    zarc_sensor s = {0};
    zarc_sensor_sample o;
    zarc_sensor_config c = {1, 2, 3, -2, -4000, 12500, 10};
    CHECK(!zarc_sensor_snapshot(&s, 0, &o));
    CHECK(!zarc_sensor_init(&s, &c, 0, 0));
    CHECK(zarc_sensor_init(&s, &c, 7, 0));
    CHECK(zarc_sensor_snapshot(&s, 0, &o) && o.quality == ZARC_SAMPLE_UNKNOWN);
    CHECK(zarc_sensor_publish(&s, 7, 1, 1, 2, -4000, ZARC_SAMPLE_VALID));
    CHECK(zarc_sensor_snapshot(&s, 2, &o) && o.value == -4000 &&
          o.unit_id == 2 && o.calibration_id == 3 && o.scale10 == -2);
    CHECK(!zarc_sensor_publish(&s, 8, 2, 2, 2, 0, ZARC_SAMPLE_VALID));
    CHECK(!zarc_sensor_publish(&s, 7, 1, 2, 2, 0, ZARC_SAMPLE_VALID));
    CHECK(!zarc_sensor_publish(&s, 7, 2, 3, 2, 0, ZARC_SAMPLE_VALID));
    CHECK(!zarc_sensor_publish(&s, 7, 2, 2, 2, 12501, ZARC_SAMPLE_VALID));
    CHECK(!zarc_sensor_publish(&s, 7, 2, 2, 2, 0, ZARC_SAMPLE_UNKNOWN));
    CHECK(zarc_sensor_snapshot(&s, 10, &o) && o.quality == ZARC_SAMPLE_VALID);
    CHECK(zarc_sensor_snapshot(&s, 11, &o) && o.quality == ZARC_SAMPLE_STALE && o.value == 0);
    CHECK(!zarc_sensor_snapshot(&s, 10, &o));
    CHECK(!zarc_sensor_publish(&s, 7, 2, 1, 11, 0, ZARC_SAMPLE_VALID));
    CHECK(zarc_sensor_publish(&s, 7, 2, 11, 11, 123, ZARC_SAMPLE_FAULT));
    CHECK(zarc_sensor_snapshot(&s, 11, &o) && o.quality == ZARC_SAMPLE_FAULT && o.value == 0);
    CHECK(zarc_sensor_publish(&s, 7, 3, 12, 12, 12500, ZARC_SAMPLE_VALID));
    CHECK(zarc_sensor_snapshot(&s, 12, &o) && o.quality == ZARC_SAMPLE_VALID && o.value == 12500);
    CHECK(zarc_sensor_init(&s, &c, 8, 0));
    CHECK(!zarc_sensor_publish(&s, 7, 4, 0, 0, 0, ZARC_SAMPLE_VALID));
    c.minimum = 1; c.maximum = 0;
    CHECK(!zarc_sensor_init(&s, &c, 9, 0));
    puts("Numeric sensor sample contracts passed");
    return 0;
}
