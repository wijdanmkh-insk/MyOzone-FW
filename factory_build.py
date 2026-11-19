Import("env")
import os

def make_factory_bin(source, target, env):
    print("\n=======================================================")
    print("[SILICON WHISPERER] Memulai Penyatuan Factory Bin...")
    print("=======================================================")

    build_dir   = env.subst("$BUILD_DIR")
    python_exe  = env.subst("$PYTHONEXE")
    esptool_bin = env.subst("$UPLOADER")

    bootloader = os.path.join(build_dir, "bootloader.bin")
    partitions = os.path.join(build_dir, "partitions.bin")
    firmware   = os.path.join(build_dir, "firmware.bin")
    out_file   = os.path.join(build_dir, "PROVEGGIES_FACTORY_V1.bin")

    # tool-esptool v5+ adalah binary ELF mandiri, sedangkan versi lama berupa .py
    if esptool_bin.endswith(".py"):
        runner = f'"{python_exe}" "{esptool_bin}"'
        pad_flag = "--fill-flash-size 4MB"
    else:
        runner = f'"{esptool_bin}"'
        pad_flag = "--pad-to-size 4MB"

    cmd = (
        f'{runner} --chip esp32 merge-bin '
        f'-o "{out_file}" --flash-mode dio --flash-freq 40m --flash-size 4MB '
        f'{pad_flag} '
        f'0x1000 "{bootloader}" 0x8000 "{partitions}" 0x10000 "{firmware}"'
    )

    ret = env.Execute(cmd)
    if ret != 0:
        print("\n[SILICON WHISPERER] GAGAL menggabungkan biner! Cek log error di atas.\n")
        env.Exit(1)

    print("\n=======================================================")
    print("[SILICON WHISPERER] BERHASIL! Senjata Siap Edar (4MB)!")
    print(f"Ambil file ini: {out_file}")
    print("=======================================================\n")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", make_factory_bin)