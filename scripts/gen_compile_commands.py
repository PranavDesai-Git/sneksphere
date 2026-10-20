#!/usr/bin/env python3
"""
Generate compile_commands.json for Clangd / Neovim without requiring bear or cmake.
Supports nested source files, dynamic flags, and absolute paths.
"""

import argparse
import json
import os
import shlex
import sys
from pathlib import Path

def parse_args():
    parser = argparse.ArgumentParser(description="Generate compile_commands.json")
    parser.add_argument("--output", default="compile_commands.json", help="Output file path")
    parser.add_argument("--cc", default="clang", help="C compiler command")
    parser.add_argument("--cflags", default="", help="Compiler and preprocessor flags")
    parser.add_argument("--src-dir", default="src", help="Source directory")
    parser.add_argument("--obj-dir", default="objects", help="Object directory")
    parser.add_argument("--sources", nargs="*", default=None, help="List of source files")
    return parser.parse_args()

def main():
    args = parse_args()
    project_root = Path.cwd().resolve()
    src_dir = (project_root / args.src_dir).resolve()
    obj_dir = (project_root / args.obj_dir).resolve()

    if args.sources:
        raw_sources = []
        for item in args.sources:
            raw_sources.extend(shlex.split(item))
        src_files = [Path(s).resolve() for s in raw_sources if s.strip()]
    else:
        src_files = sorted(src_dir.rglob("*.c"))

    # Filter out flags that clangd shouldn't be confused by (-MMD, -MP, etc.)
    raw_flags = shlex.split(args.cflags)
    filtered_flags = [f for f in raw_flags if f not in ("-MMD", "-MP") and not f.startswith("-MF")]

    commands = []
    for src in src_files:
        if not src.is_file():
            continue
        try:
            rel_path = src.relative_to(src_dir)
        except ValueError:
            rel_path = src.name

        obj_path = obj_dir / rel_path.with_suffix(".o")
        cmd_list = [args.cc] + filtered_flags + ["-c", str(src), "-o", str(obj_path)]

        commands.append({
            "directory": str(project_root),
            "command": shlex.join(cmd_list),
            "file": str(src),
            "output": str(obj_path)
        })

    # Index vendored libraries (e.g. vendor/raylib/src) so 'gd' jumps to actual C definitions
    vendor_raylib_src = project_root / "vendor" / "raylib" / "src"
    if vendor_raylib_src.is_dir():
        raylib_flags = [
            args.cc,
            f"-I{vendor_raylib_src}",
            f"-I{vendor_raylib_src / 'external' / 'glfw' / 'include'}",
            "-D_GLFW_X11",
            "-DPLATFORM_DESKTOP"
        ]
        for vsrc in sorted(vendor_raylib_src.glob("*.c")):
            vobj = vsrc.with_suffix(".o")
            vcmd = raylib_flags + ["-c", str(vsrc), "-o", str(vobj)]
            commands.append({
                "directory": str(project_root),
                "command": shlex.join(vcmd),
                "file": str(vsrc),
                "output": str(vobj)
            })

    output_path = Path(args.output).resolve()
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(commands, f, indent=2)
        f.write("\n")

if __name__ == "__main__":
    main()
