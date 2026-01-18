# Changelog

## [1.0.0] - 2026-01-18

### Added
- **PLCopen Function Blocks Library**: Initial release of the library compliant with PLCopen specifications.
- **Common Module**:
  - Buffer overflow protection (`check_overflow`)
  - Division by zero protection (`safe_divide`)
  - NaN/Inf detection (`check_nan_inf`)
  - Output clamping (`clamp_output`)
- **PID Controller (US1)**:
  - Standard PID algorithm with anti-windup (conditional integration)
  - Derivative kick avoidance (derivative on PV)
  - Bumpless transfer (Manual/Auto modes)
- **PT1 Filter (US2)**:
  - First-order logic low-pass filter
  - Configurable time constant and sample time
- **RAMP Generator (US3)**:
  - Configurable rise/fall rates
  - Smooth Setpoint generation
- **LIMIT (US4)**:
  - Min/Max output limitation with status flags
- **DEADBAND (US5)**:
  - Signal deadband processing
- **INTEGRATOR (US6)**:
  - Time-based integration with reset and limits
- **DERIVATIVE (US7)**:
  - Numerical differentiation with optional filtering
- **Testing**:
  - Comprehensive Unit Test Suite using Unity
  - Docker-based test environment
  - Code Coverage > 90%
- **Documentation**:
  - API Specifications
  - User Stories and Implementation Plans
  - Data Models
