/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

enum zmk_os_type {
  ZMK_OS_LINUX,
  ZMK_OS_MAC,
  ZMK_OS_WIN,
};

int zmk_select_preferred_os_type(const enum zmk_os_type);
int zmk_cycle_preferred_os_type(int);
enum zmk_os_type zmk_get_preferred_os_type(void);
