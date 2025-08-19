# os-layer-zmk-module

A [ZMK](https://zmk.dev) module for automatically adapting key behaviors to the connected host operating system.  
This can be useful for handling differences between Linux, macOS, and Windows key mappings.

---

## Features
- Detects the host OS (WIP) and adjusts key output accordingly.  
- Provides a flexible layer for defining OS-specific keymaps.  
---

## Installation

Add the module to your `config/west.yml`:
```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: Samsuper12                            # <--- add this
      url-base: https://github.com/Samsuper12     # <--- and this
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    - name: os-layer-zmk-module                   # <--- and these
      remote: Samsuper12                          # <---
      revision: main                              # <---
  self:
    path: config
```
---
## Usage example
```yaml
#include <dt-bindings/zmk/os_layer_defines.h>
#include <os_layer.dtsi>


/ {
    behaviors {
        ctrl_os: ctrl_os {
            compatible = "zmk,behavior-os-mod";
            #binding-cells = <0>;
            bindings = <&os_choose KP_OS_OTHER &kp LCTRL>,
                        <&os_choose KP_OS_MAC &kp LEFT_GUI>;
        };
    };

    keymap {
        compatible = "zmk,keymap";

        layer_default {
            bindings = <... &ctrl_os ...>;
        };

        layer_util {
            bindings = <... &os_sel GO_MAC_OS &os_sel GO_LINUX_OS ...>;
        };
    };
};
```
## OS-Selector node
With it you can manually change the preferred OS layer using the `&os_sel` node that can accept these arguments:

| Args | Description |
|:-------------|:---------------|
| GO_NEXT_OS   | Change current OS to the next (Linux is the first and Win is the last) |
| GO_PREV_OS   | The same but in reverse |
| GO_LINUX_OS  | Jump to the Linux OS layer  |
| GO_MAC_OS    | Jump to the macOS layer    |
| GO_WIN_OS    | Jump to the Windows OS layer    |

#### Example
```yaml
bindings = <...&os_sel GO_LINUX_OS ...>;
```

## OS-Mod ZMK behavior
This is useful when using multiple operating systems with one keyboard, as it prevents you from having to rework a keymap or break your fingers. For example, I'm struggling with the default keymap for Lotus58, which has LCTRL on the left (!), but on macOS it's used as ALT.

The `&os_choose` says that the next key binding in `zmk,behavior-os-mod` should only be triggered only if the OS layer is matches. Otherwise, it will fall into `&trans` or `&os_choose KP_OS_OTHER` if defined.

Be aware of the index if you use the `&os_sel GO_NEXT_OS / GO_PREV_OS` node.

| Args         | Index | Binding will be triggered if |
|:-------------|:----:|:-----------------|
| KP_OS_LINUX  |0     | Current OS layer is Linux |
| KP_OS_MAC    |1     | Current OS layer is macOS |
| KP_OS_WIN    |2     | Current OS layer is Windows  |
| KP_OS_OTHER  | NULL | There is no '&os_choose' node that matches the current OS layer. Kinda fallback.|

#### Example 1
```
ctrl_os: ctrl_os {
            compatible = "zmk,behavior-os-mod";
            #binding-cells = <0>;
            bindings = <&os_choose KP_OS_LINUX &kp LCTRL>,
                        <&os_choose KP_OS_MAC &kp LEFT_GUI>;
        };

```

#### Example 2 
```
ctrl_os: ctrl_os {
            compatible = "zmk,behavior-os-mod";
            #binding-cells = <0>;
            bindings = <&os_choose KP_OS_LINUX &kp LCTRL>,
                        <&os_choose KP_OS_MAC &kp LEFT_COMMAND>,
                        <&os_choose KP_OS_WIN &kp RIGHT_ALT>,
                        <&os_choose KP_OS_OTHER &kp RCTRL>;
        };

```
