#include <stdlib.h>

void assert_disk_equals(const uint8_t* expected);
void test_disk_starts_at_all_zeroes(void);
void test_read_write_round_trip(void);
void test_overwrite_existing_data(void);
void test_invalid_read_inputs_leave_disk_unchanged(void);
void test_invalid_write_inputs_leave_disk_unchanged(void);
void test_read_write_at_final_disk_byte(void);
