#include "../../src/file_system/disk.h"
#include "block_layer_test.h"
#include "disk_test.h"
#include "unity_internals.h"

void setUp(void) { disk_reset(); }

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    // disk layer tests
    RUN_TEST(test_disk_starts_at_all_zeroes);
    RUN_TEST(test_read_write_round_trip);
    RUN_TEST(test_overwrite_existing_data);
    RUN_TEST(test_invalid_read_inputs_leave_disk_unchanged);
    RUN_TEST(test_invalid_write_inputs_leave_disk_unchanged);
    RUN_TEST(test_read_write_at_final_disk_byte);

    // block layer tests
    RUN_TEST(test_block_starts_as_zeroes);
    RUN_TEST(test_block_write_read_round_trip);
    RUN_TEST(test_block_write_only_changes_target_block);
    RUN_TEST(test_block_writes_are_independent);
    RUN_TEST(test_invalid_read_inputs_preserve_state);
    RUN_TEST(test_invalid_write_inputs_preserve_state);

    return UNITY_END();
}
