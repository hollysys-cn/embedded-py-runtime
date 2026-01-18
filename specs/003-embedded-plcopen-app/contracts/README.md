# Contracts 目录说明

**Branch**: `003-embedded-plcopen-app` | **Date**: 2026-01-18

## 概述

本功能（嵌入式 PLCOpen 控制应用程序）为演示程序集，不对外提供 API 接口。

因此，本目录不包含 API 契约定义文件（如 OpenAPI、GraphQL schema 等）。

## 使用的外部契约

本功能使用以下已定义的接口：

| 来源 | 接口 | 说明 |
|------|------|------|
| 002-plcopen-function-blocks | PLCOpen 功能块 API | 通过 `#include <plcopen/plcopen.h>` 使用 |

## 内部接口

演示程序的内部接口定义在 [data-model.md](../data-model.md) 中，包括：

- `PlantModel` - 被控对象模型
- `NoiseGenerator` - 噪声发生器
- CSV 输出格式宏

这些接口仅供演示程序内部使用，不作为公开契约。
