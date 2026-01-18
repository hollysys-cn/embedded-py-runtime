/* 测试占位文件 - 待扩展 */
#include "unity.h"
#include "plcopen/fb_derivative.h"
void setUp(void) {}
void tearDown(void) {}
void test_placeholder(void) { TEST_IGNORE_MESSAGE("待实现"); }
void run_test_fb_derivative(void) { RUN_TEST(test_placeholder); }
int main(void) { UNITY_BEGIN(); run_test_fb_derivative(); return UNITY_END(); }
