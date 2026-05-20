Import("env")

import os
import shutil


def merge_firmware(source, target, env):
    board = env.BoardConfig()
    build_dir   = env.subst("$BUILD_DIR")
    prog_name   = env.subst("$PROGNAME")
    project_dir = env.subst("$PROJECT_DIR")
    chip        = board.get("build.mcu", "esp32s3")

    firmware_bin = os.path.join(build_dir, f"{prog_name}.bin")
    merged_bin   = os.path.join(build_dir, f"{prog_name}-merged.bin")

    flash_mode = board.get("build.flash_mode", "dio")
    flash_freq = str(board.get("build.f_flash", "80m")).rstrip("L")
    if flash_freq.isdigit():
        flash_freq = f"{int(flash_freq) // 1_000_000}m"
    flash_size = board.get("upload.flash_size", "8MB")

    args = [
        "--chip", chip,
        "merge_bin",
        "-o", merged_bin,
        "--flash_mode", flash_mode,
        "--flash_freq", flash_freq,
        "--flash_size", flash_size,
    ]
    for addr, img in env.get("FLASH_EXTRA_IMAGES", []):
        args += [addr, img]
    args += ["0x10000", firmware_bin]

    pkg_dir    = env.PioPlatform().get_package_dir("tool-esptoolpy") or ""
    esptool_py = os.path.join(pkg_dir, "esptool.py")
    python_exe = env.subst("$PYTHONEXE")

    cmd = f'"{python_exe}" "{esptool_py}" ' + " ".join(f'"{a}"' for a in args)
    rc = env.Execute(cmd)
    if rc != 0:
        return

    if os.path.exists(merged_bin):
        out = os.path.join(project_dir, "firmware.bin")
        shutil.copy2(merged_bin, out)
        print("")
        print(f">>> Single-file firmware: {out}")
        print(">>> Flash it:")
        print(f"    esptool.py --chip {chip} --port /dev/cu.usbmodem* write_flash 0x0 firmware.bin")
        print("")


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_firmware)
