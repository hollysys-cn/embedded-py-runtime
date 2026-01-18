/* 测试占位文件 - 待扩展 */
#include "unity.h"
#include "plcopen/fb_ramp.h"
void setUp(void) {}
void tearDown(void) {}
void test_placeholder(void) { TEST_IGNORE_MESSAGE("待实现"); }
void run_test_fb_ramp(void) { RUN_TEST(test_placeholder); }
int main(void) { UNITY_BEGIN(); run_test_fb_ramp(); return UNITY_END(); }
