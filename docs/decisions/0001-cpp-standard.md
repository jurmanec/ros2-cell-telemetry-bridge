# ADR 0001 — C++ standard for edge

**Status:** Accepted  
**Date:** 2026-07-13  
**Milestone:** M0

## Decision

Use **C++20** for edge packages (`cell_sim` and follow-ons).

## Context

Charter emphasizes modern C++. ROS 2 Jazzy supports compiling nodes at C++17+; C++20 is widely available in the Jazzy/Ubuntu 24.04 toolchain and is a practical modern dialect without requiring C++23 availability everywhere.

## Consequences

- `CMAKE_CXX_STANDARD 20` in edge package `CMakeLists.txt`.
- Prefer C++20 library features where they clarify code; no mandate to use every C++20 facility.
- Revisit C++23 only if a concrete need appears.
