/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/class/hid.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usb_ch9.h>
#include <zephyr/usb/usb_device.h>
#include <zmk/hid.h>

#include <zephyr/settings/settings.h>
#include <zmk/events/preferred_os_state_changed.h>
#include <zmk/os_layer.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static enum zmk_os_type preferred_os_type = ZMK_OS_OTHER;

#if IS_ENABLED(CONFIG_SETTINGS)
static void os_type_save_preferred_work(struct k_work *work) {
  settings_save_one("os_type/preferred", &preferred_os_type,
                    sizeof(preferred_os_type));
}

static struct k_work_delayable os_type_save_work;
#endif

static int os_type_save_preferred(void) {
#if IS_ENABLED(CONFIG_SETTINGS)
  return k_work_reschedule(&os_type_save_work,
                           K_MSEC(CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE));
#else
  return 0;
#endif
}

#if IS_ENABLED(CONFIG_SETTINGS)

static int os_type_handle_set(const char *name, size_t len,
                              settings_read_cb read_cb, void *cb_arg) {
  LOG_DBG("Setting os type value %s", name);

  if (settings_name_steq(name, "preferred", NULL)) {
    if (len != sizeof(enum zmk_os_type)) {
      LOG_ERR("Invalid os type enum size (got %d expected %d)", len,
              sizeof(enum zmk_os_type));
      return -EINVAL;
    }

    int err = read_cb(cb_arg, &preferred_os_type, sizeof(preferred_os_type));
    if (err <= 0) {
      LOG_ERR("Failed to read preferred os type from settings (err %d)", err);
      return err;
    }
  }

  return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(os_type, "os_type", NULL, os_type_handle_set,
                               NULL, NULL);

#endif /* IS_ENABLED(CONFIG_SETTINGS) */

int zmk_select_preferred_os_type(enum zmk_os_type os) {
  preferred_os_type = os;
  os_type_save_preferred();

  raise_zmk_preferred_os_state_changed(
      (struct zmk_preferred_os_state_changed){.os = preferred_os_type});

  return 0;
}

enum zmk_os_type zmk_get_preferred_os_type(void) { return preferred_os_type; }

int zmk_calc_next_os_type(int i) {
  if ((preferred_os_type + i) > ZMK_OS_WIN) {
    return ZMK_OS_LINUX;
  } else if ((preferred_os_type + i) < ZMK_OS_LINUX) {
    return ZMK_OS_WIN;
  }

  return preferred_os_type + i;
}

//-----------------------------------------------------------------------------//

static bool fake_hid_shutdown = false;
static bool fake_hid_ready = false;

static void fake_iface_ready(const struct device *dev, const bool ready) {
  LOG_INF("Fake HID device %s interface is %s", dev->name,
          ready ? "ready" : "not ready");
  fake_hid_ready = ready;
}

static int fake_get_report(const struct device *dev, const uint8_t type,
                           const uint8_t id, const uint16_t len,
                           uint8_t *const buf) {

  LOG_DBG("HID Report.");

  return 0;
}

struct hid_device_ops fake_ops = {
    .iface_ready = fake_iface_ready,
    .get_report = fake_get_report,
};

static int zmk_usb_os_detector_init(void) {
#if IS_ENABLED(CONFIG_SETTINGS)
  k_work_init_delayable(&os_type_save_work, os_type_save_preferred_work);
#endif

#if IS_ENABLED(CONFIG_ZMK_OS_LAYER_DETECT_VIA_USB)
  const struct device *hid_dev;
  int ret;

  hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
  if (!device_is_ready(hid_dev)) {
    LOG_ERR("HID Device is not ready");
    return -EIO;
  }

  ret = hid_device_register(hid_dev, zmk_hid_report_desc,
                            sizeof(zmk_hid_report_desc), &fake_ops);
  if (ret != 0) {
    LOG_ERR("Failed to register HID Device, %d", ret);
    return ret;
  }

  while (true) {
    if (fake_hid_shutdown) {
      break;
    }

    if (!fake_hid_ready) {
      LOG_DBG("USB HID device is not ready");
      continue;
    }
  }

#endif

  return 0;
}

SYS_INIT(zmk_usb_os_detector_init, APPLICATION,
         CONFIG_APPLICATION_INIT_PRIORITY);