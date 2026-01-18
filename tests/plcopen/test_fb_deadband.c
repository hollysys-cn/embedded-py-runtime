/* 测试占位文件 */
#include "unity.h"
#include "plcopen/fb_deadband.h"
void setUp(void) {}
void tearDown(void) {}
void test_placeholder(void) { TEST_IGNORE_MESSAGE("待实现"); }
int main(void) { UNITY_BEGIN(); RUN_TEST(test_placeholder); return UNITY_END(); }
