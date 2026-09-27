#!/usr/bin/env python3
# Copyright (c) 2026, Niklas Hauser
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Generates the header sets and library sources from the GCC releases.
See docs/generating.md for a detailed explanation.

avr-gcc is built without libstdc++, however, the libstdc++ configure script
supports avr-libc. We run it against avr-libc and install only the headers,
which generates bits/c++config.h with what avr-libc supports. Each set is
generated from the GCC release of the newest avr-gcc that it supports, which
also configures libstdc++. The generated headers are checked by compiling
them with the oldest avr-gcc that the set is supposed to support.

The compilers are found via Homebrew (osx-cross/avr tap) or can be set via
the AVR_GXX_<version> environment variables, for example
AVR_GXX_14=/opt/avr/bin/avr-g++. The GCC sources are cached in
$GCC_SRC_CACHE (default: /tmp/avr-libstdcpp).
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Newest and oldest avr-gcc version of each header set.
# See docs/compatibility.md for why these sets were chosen.
SETS = {
    10: 8,   # avr-gcc 8, 9, 10 (C++17 only for avr-gcc 8, 9)
    12: 11,  # avr-gcc 11, 12
    13: 13,  # avr-gcc 13
    15: 14,  # avr-gcc 14, 15
}

# Only the libstdc++ sources and what its configure script needs
SPARSE = ["/libstdc++-v3/", "/config/", "/include/", "/libgcc/gthr*", "/gcc/BASE-VER",
          "/gcc/DATESTAMP", "/gcc/DEV-PHASE", "/config.guess", "/config.sub", "/install-sh",
          "/mkinstalldirs", "/missing", "/move-if-change", "/config-ml.in", "/ltmain.sh",
          "/libtool.m4"]

# C99 math functions that are forwarded to the compiler builtins if avr-libc lacks them
BUILTINS = ["acosh", "asinh", "atanh", "cbrt", "erf", "erfc", "exp2", "expm1", "lgamma",
            "log1p", "log2", "logb", "nearbyint", "rint", "round", "tgamma", "trunc"]


def read(path):
    return path.read_text(encoding="utf-8", errors="surrogateescape")


def write(path, text):
    path.write_text(text, encoding="utf-8", errors="surrogateescape")


def patch(path, pattern, repl, count=0, flags=re.M, required=True):
    text, n = re.subn(pattern, repl, read(path), count=count, flags=flags)
    if required and not n:
        sys.exit(f"Failed to patch {path}: {pattern}")
    write(path, text)


def files(path):
    return (f for f in sorted(path.rglob("*")) if f.is_file())


def compiler(version):
    if cxx := os.environ.get(f"AVR_GXX_{version}"):
        return Path(cxx)
    prefix = subprocess.run(["brew", "--prefix", f"avr-gcc@{version}"], check=True,
                            capture_output=True, text=True).stdout.strip()
    return Path(prefix) / "bin/avr-g++"


def version(cxx, option="-dumpfullversion"):
    return subprocess.run([cxx, option], check=True, capture_output=True, text=True).stdout.strip()


def supports(cxx, *flags):
    return subprocess.run([cxx, *flags, "-E", "-x", "c++", os.devnull],
                          capture_output=True).returncode == 0


def configure(cxx):
    """Configures libstdc++ of the GCC release of cxx for avr-libc and installs the headers."""
    release = version(cxx)
    cache = Path(os.environ.get("GCC_SRC_CACHE", "/tmp/avr-libstdcpp"))
    gcc = cache / f"gcc-{release}"
    build = cache / f"build-{release}"
    if not gcc.exists():
        subprocess.run(["git", "-c", "advice.detachedHead=false", "clone", "--quiet", "--depth", "1",
                        "--filter=blob:none", "--sparse", "--branch", f"releases/gcc-{release}",
                        "https://github.com/gcc-mirror/gcc.git", gcc], check=True)
        subprocess.run(["git", "-C", gcc, "sparse-checkout", "set", "--no-cone", *SPARSE], check=True)

    shutil.rmtree(build, ignore_errors=True)
    build.mkdir(parents=True)
    bindir = cxx.parent
    env = dict(os.environ, PATH=f"{bindir}:{os.environ['PATH']}")
    build_triple = subprocess.run([gcc / "config.guess"], check=True, capture_output=True,
                                  text=True).stdout.strip()
    with open(build / "configure.log", "w") as log:
        for cmd in ([gcc / "libstdc++-v3/configure", "--host=avr", f"--build={build_triple}",
                     f"--prefix={build}/install", "--disable-shared", "--disable-multilib",
                     "--disable-libstdcxx-pch", "--disable-nls", f"CC={bindir}/avr-gcc", f"CXX={cxx}",
                     f"AR={bindir}/avr-ar", f"RANLIB={bindir}/avr-ranlib"],
                    ["make", "-C", "include", "install"],
                    ["make", "-C", "libsupc++", "install-data"]):
            if subprocess.run(cmd, cwd=build, env=env, stdout=log, stderr=log).returncode:
                sys.exit(f"Configuring libstdc++ failed, see {log.name}")
    return gcc / "libstdc++-v3", build / f"install/include/c++/{release}", release


def generate(newest, oldest, out):
    lib, src, release = configure(newest)
    avr_version = int(version(oldest, "-dumpversion").split(".")[0])
    print(f"Generating from {src}")
    dst = out / "include"
    shutil.rmtree(dst, ignore_errors=True)
    shutil.rmtree(out / "src", ignore_errors=True)
    (out / "src").mkdir(parents=True)
    shutil.copytree(src, dst)
    # Flatten the target directory into the generic headers
    for sub in ("bits", "ext"):
        shutil.copytree(dst / "avr" / sub, dst / sub, dirs_exist_ok=True)
    shutil.rmtree(dst / "avr")

    # The out-of-line parts of the containers, which are usually compiled into
    # libstdc++, plus our own implementations of what must be adapted to AVR.
    for source in ("src/c++98/list.cc", "src/c++98/tree.cc", "src/c++11/hashtable_c++0x.cc",
                   "libsupc++/new_handler.cc"):
        shutil.copy(lib / source, out / "src")
    for source in (ROOT / "tools/src").glob("*.cc"):
        shutil.copy(source, out / "src")
    # Rename the included source so that it is not compiled on its own
    shutil.copy(lib / "src/shared/hashtable-aux.cc", out / "src/hashtable-aux.h")
    patch(out / "src/hashtable_c++0x.cc", r'"\.\./shared/hashtable-aux\.cc"', '"hashtable-aux.h"')

    # Remove extensions and TS that are not useful on AVR
    for name in ("decimal", "experimental", "parallel", "tr1", "tr2", "ext/pb_ds"):
        shutil.rmtree(dst / name, ignore_errors=True)
    # Remove the sources of the std module, which require the compiled library,
    # and the C compatibility headers that clash with avr-libc
    for name in ("bits/std.cc", "bits/std.compat.cc", "complex.h", "fenv.h", "math.h",
                 "stdlib.h", "tgmath.h"):
        (dst / name).unlink(missing_ok=True)
    # avr-libc defines abs() as a macro, which breaks <chrono> if <cstdlib>
    # is not included. This wrapper only removes the macro, but does not declare
    # the overloads of std::abs() in the global namespace like the libstdc++ one.
    write(dst / "stdlib.h", "// avr-libstdcpp: removes the abs() and labs() macros of avr-libc\n"
          "#pragma GCC system_header\n#include_next <stdlib.h>\n#undef abs\n#undef labs\n")

    config = dst / "bits/c++config.h"
    # The configure checks for C99 math fail, since avr-libc lacks some functions,
    # however, most of them exist and the missing ones are removed further below.
    # avr-libc does not define float_t and double_t, they are defined further below.
    for macro, required in (("_GLIBCXX98_USE_C99_MATH", True), ("_GLIBCXX11_USE_C99_MATH", True),
                            ("_GLIBCXX_USE_C99_MATH_TR1", True), ("_GLIBCXX_USE_C99_MATH_FUNCS", False),
                            ("_GLIBCXX_HAVE_C99_FLT_EVAL_TYPES", False)):
        patch(config, rf"^/\* #undef ({macro}) \*/", r"#define \1 1 // avr-libstdcpp", required=required)
    # The special math functions (std::beta, ...) are implemented in the TR1 headers,
    # which are removed above
    patch(config, r"^(#\s*define _GLIBCXX_USE_STD_SPEC_FUNCS) 1", r"\1 0")
    # GCC 13 and 14.1 crash on the std::float32_t overloads, since float and
    # double are both 32-bit on AVR.
    if avr_version in (13, 14):
        patch(config, r"(#define _GLIBCXX_CXX_CONFIG_H 1\n)", r"\1\n// avr-libstdcpp: avoid internal "
              r"compiler error in GCC 13 and 14.1\n#if __GNUC__ == 13 || (__GNUC__ == 14 && __GNUC_MINOR__ < 2)"
              r"\n#undef __STDCPP_FLOAT32_T__\n#endif\n", count=1)

    # avr-gcc < 9 cannot parse attributes after if statements
    if avr_version < 9:
        for header in files(dst):
            patch(header, r"\)[ \t]*\[\[__(?:un)?likely__\]\]", ")", required=False)
    # avr-gcc < 12 cannot parse the C++23 decay-copy auto(x), which GCC 12.5 backported
    if avr_version < 12:
        for header in files(dst):
            patch(header, r"\bauto\((__\w+)\)", r"std::decay_t<decltype(\1)>(\1)", required=False)

    # libstdc++ instantiates std::string in its compiled library, so allow the
    # compiler to instantiate the used members instead. The other extern templates
    # are kept, for example for the locale facets, which would otherwise add a
    # static initializer to every translation unit.
    string = dst / "bits/basic_string.tcc"
    patch(string, r"\n(\s*)(extern template class basic_string<char>;)",
          r"\n\1// avr-libstdcpp: \2", count=1)
    patch(string, r"\n(\s*)(extern template void\n\s*basic_string<char>::_M_replace_cold\([^;]*;)",
          r"\n#if 0 // avr-libstdcpp: instantiated implicitly\n\1\2\n#endif", count=1, required=False)

    # Before GCC 13 std::errc::value_too_large requires EOVERFLOW
    patch(dst / "bits/error_constants.h", r"(\n(\s*)value_too_large = \s*EOVERFLOW,\n)(#endif)",
          r"\1#elif defined __AVR__\n\2value_too_large = 999,\n\3", count=1, required=False)

    cmath = dst / "cmath"
    # avr-libc < 2.1 defines the float math functions as macros, src/math.cc
    # implements them as functions, so they must not be defined as macros.
    functions = re.findall(r"^\s+(float|long|int) ([a-z0-9]+f)\((.*)\)$", read(ROOT / "tools/src/math.cc"), re.M)
    undefs = "".join(f"#undef {name}\n" for _, name, _ in functions)
    decls = "".join(f"{ret} {name}({args});\n" for ret, name, args in functions)
    # avr-libc < 2.2 also lacks some long double functions, which are declared
    # here, so that the headers do not depend on the avr-libc version.
    ldecls = "".join(f"#ifndef {name[:-1]}l\n{ret.replace('float', 'long double')} {name[:-1]}l"
                     f"({args.replace('float', 'long double')});\n#endif\n"
                     for ret, name, args in functions if ret != "int" and name != "squaref")
    patch(cmath, r"(#include_next <math.h>\n.*?\n)", lambda m: f"""{m[1]}
// avr-libstdcpp: see src/math.cc
{undefs}#include <avr/version.h>
extern "C"
{{
#if __AVR_LIBC_VERSION__ < 20100UL
{decls}#endif
#if __AVR_LIBC_VERSION__ < 20200UL
{ldecls}#endif
}}
""", count=1, flags=re.S)
    # avr-libc does not define the floating-point classification macros, which
    # std::fpclassify() passes to __builtin_fpclassify() (see modm-io/avr-libstdcpp#41)
    patch(cmath, r"(#undef isunordered\n)", lambda m: m[1] + """
// avr-libstdcpp: avr-libc does not define these
#if !defined(FP_NAN) && !defined(FP_INFINITE) && !defined(FP_ZERO) \\
  && !defined(FP_SUBNORMAL) && !defined(FP_NORMAL)
#define FP_NAN         0
#define FP_INFINITE    1
#define FP_ZERO        2
#define FP_SUBNORMAL   3
#define FP_NORMAL      4
#elif !defined(FP_NAN) || !defined(FP_INFINITE) || !defined(FP_ZERO) \\
  || !defined(FP_SUBNORMAL) || !defined(FP_NORMAL)
#error "Some floating-point number classification macros are missing."
#error "Either define all of them or define none of them so that <cmath> can do it."
#endif
""", count=1)
    # avr-libc does not define these types, however, FLT_EVAL_METHOD is 0 on AVR
    patch(cmath, r"^([ \t]*)using ::(float|double)_t;", r"\1typedef \2 \2_t; // avr-libstdcpp")

    # The headers are usually installed in a system path, however, we are often
    # included via -I, so we must always mark them as system headers.
    for header in files(dst):
        text = re.sub(r"#ifdef _GLIBCXX_SYSHDR\n(#pragma GCC system_header)\n#endif", r"\1", read(header))
        if "#pragma GCC system_header" not in text:
            text = "#pragma GCC system_header // avr-libstdcpp\n" + text
        write(header, text)

    # Remove all imports of C library names that avr-libc does not declare by
    # compiling all headers and commenting out the offending using declarations.
    std = next(s for s in ("c++26", "c++23", "c++20", "c++17") if supports(oldest, f"-std={s}"))
    flags = ["-fno-exceptions", "-fno-rtti", "-mmcu=atmega328p", "-nostdinc++", f"-I{dst}",
             "-fsyntax-only", "-x", "c++"]
    headers = sorted(h.name for h in dst.iterdir() if h.is_file() and h.suffix != ".h")
    if supports(oldest, "-fcoroutines"):
        flags.append("-fcoroutines")
    else:
        headers.remove("coroutine")
    with tempfile.NamedTemporaryFile("w", suffix=".cpp") as all_headers:
        all_headers.write("".join(f"#include <{h}>\n" for h in headers))
        all_headers.flush()

        def check(std=std):
            return subprocess.run([oldest, f"-std={std}", *flags, all_headers.name],
                                  capture_output=True, text=True).stderr

        for _ in range(5):
            missing = set(re.findall(r"error: '::(\w+)' has not been declared", errors := check()) +
                          re.findall(r"error: '(\w+)' has not been declared in '::'", errors))
            if not missing:
                break
            for header in files(dst):
                patch(header, r"^([ \t]*)(using ::(\w+);)", required=False,
                      repl=lambda m: f"{m[1]}// avr-libstdcpp: {m[2]}" if m[3] in missing else m[0])
        # Without the double overloads of the C99 math functions, calls with double
        # arguments are ambiguous, so forward them to the compiler builtins instead.
        patch(cmath, rf"^([ \t]*)// avr-libstdcpp: using ::({'|'.join(BUILTINS)});",
              r"\1inline double \2(double __x) { return __builtin_\2(__x); } // avr-libstdcpp")

        errors = [line for line in (check() + check("c++17")).splitlines() if "error:" in line]
        if errors:
            sys.exit("\n".join(errors) + "\nGenerated headers do not compile!")


def redirect(sets):
    """Generates include and src, which redirect every file to the header set of
    the avr-gcc version, so that all compilers only need one include path."""
    for path in ROOT.glob("gcc*"):
        if path.is_dir() and path.name not in (f"gcc{newest}" for newest in sets):
            shutil.rmtree(path)
    names = sorted({str(f.relative_to(ROOT / f"gcc{newest}")) for newest in sets
                    for f in [*files(ROOT / f"gcc{newest}/include"), *files(ROOT / f"gcc{newest}/src")]
                    if not (f.parent.name == "src" and f.suffix == ".h")})
    for sub in ("include", "src"):
        shutil.rmtree(ROOT / sub, ignore_errors=True)
    for name in names:
        file = ROOT / name
        file.parent.mkdir(parents=True, exist_ok=True)
        # Identical in all sets and uses #include_next, which only works for
        # headers found via the include path, so it cannot be redirected.
        if name == "include/stdlib.h":
            shutil.copy(ROOT / f"gcc{max(sets)}" / name, file)
            continue
        text = "" if name.endswith(".cc") else "#pragma GCC system_header // avr-libstdcpp\n"
        for index, newest in enumerate(sorted(sets, reverse=True)):
            text += f"#{'el' if index else ''}if __GNUC__ >= {sets[newest]}\n"
            if (ROOT / f"gcc{newest}" / name).exists():
                text += f'#include "{os.path.relpath(ROOT / f"gcc{newest}" / name, file.parent)}"\n'
            else:
                text += f'#error "<{name.split("/", 1)[1]}> is not available with this avr-gcc version"\n'
        write(file, text + f'#else\n#error "avr-libstdcpp requires avr-gcc {min(sets.values())} or newer"\n#endif\n')


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    parser.add_argument("sets", nargs="*", metavar="NEWEST:OLDEST",
                        help="avr-gcc versions of the sets to generate (default: all), "
                             "for example 16:15 generates gcc16 for avr-gcc 15 and 16.")
    args = parser.parse_args()
    sets = dict(map(int, s.split(":")) for s in args.sets) if args.sets else SETS
    for newest, oldest in sets.items():
        generate(compiler(newest), compiler(oldest), ROOT / f"gcc{newest}")
    redirect(SETS | sets)
