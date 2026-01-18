==========================================
  Running PLCopen Performance Tests
==========================================
Compiling Performance Tests...
Running Performance Tests...
警告：非 STM32 环境，性能测试为模拟值

性能基准测试 (1000 次迭代):
  [PID] 平均执行时间: 0.00 μs
tests/plcopen/test_performance.c:101:test_performance_pid:PASS
  [PT1] 平均执行时间: 0.00 μs
tests/plcopen/test_performance.c:102:test_performance_pt1:PASS
  [RAMP/LIMIT/DEADBAND/INTEGRATOR/DERIVATIVE] 所有功能块预期 < 10μs
tests/plcopen/test_performance.c:93:test_performance_all_fb:INFO: \xE9\x9C\x80\xE8\xA6\x81\xE5\x9C\xA8\xE7\x9B\xAE\xE6\xA0\x87\xE7\xA1\xAC\xE4\xBB\xB6\xE4\xB8\x8A\xE8\xBF\x9B\xE8\xA1\x8C\xE8\xAF\xA6\xE7\xBB\x86\xE6\x80\xA7\xE8\x83\xBD\xE6\xB5\x8B\xE8\xAF\x95
tests/plcopen/test_performance.c:103:test_performance_all_fb:PASS

-----------------------
3 Tests 0 Failures 0 Ignored 
OK
