/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_ch9.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/settings/settings.h>
#include <zmk/events/preferred_os_state_changed.h>
#include <zmk/os_detector.h>

LOG_MODULE_REGISTER(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_PREFERRED_OS_LINUX)
static enum zmk_os_type preferred_os_type = ZMK_OS_LINUX;
#elif IS_ENABLED(CONFIG_ZMK_PREFERRED_OS_MAC)
static enum zmk_os_type preferred_os_type = ZMK_OS_MAC;
#elif IS_ENABLED(CONFIG_ZMK_PREFERRED_OS_WIN)
static enum zmk_os_type preferred_os_type = ZMK_OS_WIN;
#else
#error "Set a default OS"
#endif

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

int zmk_cycle_preferred_os_type(int i) {
  if ((preferred_os_type + i) > ZMK_OS_WIN) {
    preferred_os_type = ZMK_OS_LINUX;
  } else if ((preferred_os_type + i) < ZMK_OS_LINUX) {
    preferred_os_type = ZMK_OS_WIN;
  } else {
    preferred_os_type++;
  }

  os_type_save_preferred();

  raise_zmk_preferred_os_state_changed(
      (struct zmk_preferred_os_state_changed){.os = preferred_os_type});
  return 0;
}

/*
static int my_control_handler(struct usb_setup_packet *setup, int32_t *len,
                              uint8_t **data) {

#if IS_ENABLED(CONFIG_SETTINGS)
  k_work_init_delayable(&os_type_save_work, os_type_save_preferred_work);
#endif

  LOG_INF("USB control: bRequest=0x%02x wLength=%u", setup->bRequest,
          setup->wLength);

  // You can act only on vendor requests, for example:
  if ((setup->bmRequestType & USB_REQTYPE_TYPE_MASK) == USB_REQTYPE_VENDOR) {
    // Do something with wLength...
    return 0; // 0 = handled
  }

  return -ENOTSUP; // pass to other handlers
}

*/

static int zmk_usb_os_detector_init(void) {
#if IS_ENABLED(CONFIG_SETTINGS)
  k_work_init_delayable(&os_type_save_work, os_type_save_preferred_work);
#endif

  // Register handler after USB is ready
  // usb_ep_register_request_handler(USB_CONTROL_EP_IN, my_control_handler,
  // NULL); usb_ep_register_request_handler(USB_CONTROL_EP_OUT,
  // my_control_handler, NULL);
  return 0;
}

SYS_INIT(zmk_usb_os_detector_init, APPLICATION,
         CONFIG_APPLICATION_INIT_PRIORITY);