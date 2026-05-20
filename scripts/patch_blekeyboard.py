Import("env")

import os


# Patches the vendored wakwak-koba/ESP32-NimBLE-Keyboard so it builds and
# actually advertises a working HID service on NimBLE-Arduino 2.x.
#
# Both edits target files inside .pio/libdeps/<env>/ESP32 BLE Keyboard/src/,
# which PlatformIO re-fetches whenever the lib is reinstalled — so we apply
# them as a pre-build step. Each edit checks for its sentinel first and is a
# no-op when the patch is already in place.
#
# Patches:
#   1. BleKeyboard.h  — add `#include <functional>` (the header uses
#      `std::function` without including it).
#   2. BleKeyboard.cpp — add `pServer->start();` right after
#      `hid->startServices();`. In NimBLE-Arduino 2.x, `startServices()` is a
#      deprecated no-op; the HID GATT service is only registered when the
#      server starts, and without it macOS sees the device but won't pair it
#      as a keyboard.


PATCHES = [
    {
        "file": "BleKeyboard.h",
        "anchor": "#include <Print.h>",
        "insert_after": "\n#include <functional>",
        "sentinel": "#include <functional>",
    },
    {
        "file": "BleKeyboard.cpp",
        "anchor": "hid->startServices();",
        "insert_after": "\n  pServer->start();",
        "sentinel": "pServer->start();",
    },
]


def _libdeps_path(env):
    libdeps_dir = env.subst("$PROJECT_LIBDEPS_DIR")
    pio_env     = env.subst("$PIOENV")
    return os.path.join(libdeps_dir, pio_env, "ESP32 BLE Keyboard", "src")


def _patch_one(path, patch):
    if not os.path.exists(path):
        print(f"[patch_blekeyboard] skip {patch['file']}: not yet installed")
        return
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
    if patch["sentinel"] in content:
        return  # already patched
    if patch["anchor"] not in content:
        print(f"[patch_blekeyboard] WARN {patch['file']}: anchor not found, "
              f"upstream layout may have changed")
        return
    patched = content.replace(
        patch["anchor"],
        patch["anchor"] + patch["insert_after"],
        1,
    )
    with open(path, "w", encoding="utf-8") as f:
        f.write(patched)
    print(f"[patch_blekeyboard] patched {patch['file']}")


def run(*_args, **_kwargs):
    base = _libdeps_path(env)
    for patch in PATCHES:
        _patch_one(os.path.join(base, patch["file"]), patch)


run()
