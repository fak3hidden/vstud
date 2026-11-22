#!/usr/bin/env bash
#
# Builds Stud's launcher from this checkout into a runnable Linux executable.
#
# Usage: ./build.sh [options]
#
#   -b, --build-dir DIR   build into DIR           (default: build)
#   -j, --jobs N          compile with N jobs      (default: every core)
#   -d, --debug           build with debug symbols  (default: RelWithDebInfo)
#       --clean           remove the build directory first
#       --tests           build the tests and run them
#       --no-rosemary     skip building Rosemary, which needs Rust and network
#   -r, --run             start Stud once it is built
#   -h, --help            this text
#
# Writes build/ui/stud-ui, which runs against this machine's Qt and SDL. For a
# single file that carries its own libraries, see packaging/build-appimage.sh.
#
# Extra CMake flags go in STUD_CMAKE_ARGS, as the packaging scripts use it:
#   STUD_CMAKE_ARGS="-DSTUD_VERSION=2.0.5" ./build.sh

set -euo pipefail

cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

# The comment block at the top of this file is the usage text.
usage() { sed -n '3,/^$/p' "$0" | sed 's/^#\{0,1\} \{0,1\}//'; }

say() { printf '\033[1mbuild:\033[0m %s\n' "$*" >&2; }
warn() { printf '\033[1;33mbuild:\033[0m %s\n' "$*" >&2; }
die() { printf '\033[1;31mbuild:\033[0m %s\n' "$*" >&2; exit 1; }

# The packages each distribution needs, as the README lists them.
packages_for_host() {
    if command -v dnf >/dev/null 2>&1; then
        echo 'sudo dnf install cmake ninja-build gcc-c++ git cargo glslc qt6-qtbase-devel \'
        echo '  libcurl-devel openssl-devel libsecret-devel zlib-devel systemd-devel \'
        echo '  libxkbcommon-devel SDL3 wpewebkit'
    elif command -v apt-get >/dev/null 2>&1; then
        echo 'sudo apt install cmake ninja-build build-essential git glslc qt6-base-dev \'
        echo '  libgles-dev libcurl4-openssl-dev libssl-dev libsecret-1-dev zlib1g-dev \'
        echo '  libudev-dev libxkbcommon-dev libsdl3-0 libwpewebkit-2.0-1'
    elif command -v pacman >/dev/null 2>&1; then
        echo 'sudo pacman -S cmake ninja base-devel git cargo shaderc qt6-base \'
        echo '  curl openssl libsecret zlib libxkbcommon sdl3 wpewebkit'
    elif command -v zypper >/dev/null 2>&1; then
        echo 'sudo zypper install cmake ninja gcc-c++ git cargo glslc qt6-base-devel \'
        echo '  libcurl-devel libopenssl-devel libsecret-devel zlib-devel libxkbcommon-devel \'
        echo '  SDL3-devel wpewebkit'
    else
        echo '(install cmake 3.20 or newer, ninja or make, a C++20 compiler, git, Qt 6.9'
        echo ' with DBus and Network, OpenSSL, libsecret, zlib and SDL 3)'
    fi
}

build_dir="build"
jobs="$(nproc 2>/dev/null || echo 4)"
build_type="RelWithDebInfo"
with_tests=0
with_rosemary=1
clean=0
run_after=0

while [ $# -gt 0 ]; do
    case "$1" in
        -b|--build-dir) [ $# -ge 2 ] || die "$1 needs a directory"; build_dir="$2"; shift 2 ;;
        -j|--jobs) [ $# -ge 2 ] || die "$1 needs a number"; jobs="$2"; shift 2 ;;
        -d|--debug) build_type="Debug"; shift ;;
        --clean) clean=1; shift ;;
        --tests) with_tests=1; shift ;;
        --no-rosemary) with_rosemary=0; shift ;;
        -r|--run) run_after=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) die "unknown option $1 (try --help)" ;;
    esac
done

# ---------------------------------------------------------------------------
# What the build needs. Each check names the thing to install rather than
# letting CMake fail halfway with a shorter message.

case "$jobs" in
    ''|*[!0-9]*) die "--jobs needs a number, not \"$jobs\"" ;;
esac

command -v cmake >/dev/null 2>&1 || {
    warn "cmake is not installed:"
    packages_for_host >&2
    exit 1
}
cmake_version="$(cmake --version | awk 'NR==1 {print $3}')"
cmake_needed=3.20
if [ "$(printf '%s\n%s\n' "$cmake_needed" "$cmake_version" | sort -V | head -1)" != "$cmake_needed" ]; then
    die "cmake $cmake_version is too old: 3.20 or newer is needed"
fi

command -v git >/dev/null 2>&1 || die "git is needed: the build fetches miniz, and Rosemary"
for compiler in c++ g++ clang++; do
    command -v "$compiler" >/dev/null 2>&1 && break
done || {
    warn "no C++ compiler found:"
    packages_for_host >&2
    exit 1
}

generator=()
if command -v ninja >/dev/null 2>&1; then
    generator=(-G Ninja)
else
    warn "ninja is not installed; using make, which is slower"
fi

# Rosemary runs the Roblox client and is built from Rust. Without it Stud still
# builds, and finds a Rosemary on PATH or in STUD_ROSEMARY at launch.
if [ "$with_rosemary" -eq 1 ] && ! command -v cargo >/dev/null 2>&1; then
    warn "cargo is not installed, so Rosemary will not be built; Roblox needs one to run"
    warn "install Rust from https://rustup.rs, or pass --no-rosemary to say so"
    with_rosemary=0
fi

if [ "$clean" -eq 1 ] && [ -d "$build_dir" ]; then
    say "removing $build_dir"
    rm -rf "$build_dir"
fi

# ---------------------------------------------------------------------------
# Configure and build.

cmake_args=(
    -S . -B "$build_dir"
    -DCMAKE_BUILD_TYPE="$build_type"
    -DSTUD_BUILD_ROSEMARY="$([ "$with_rosemary" -eq 1 ] && echo ON || echo OFF)"
    -DSTUD_BUILD_TESTS="$([ "$with_tests" -eq 1 ] && echo ON || echo OFF)"
)
# STUD_CMAKE_ARGS passes extra flags, split as a shell would; unset is empty.
# shellcheck disable=SC2206
cmake_args+=(${STUD_CMAKE_ARGS:-})

say "configuring ($build_type, rosemary $([ "$with_rosemary" -eq 1 ] && echo built || echo skipped))"
cmake "${generator[@]}" "${cmake_args[@]}"

say "building with $jobs job(s)"
cmake --build "$build_dir" -j"$jobs"

launcher="$(find "$build_dir" -maxdepth 2 -type f -name stud-ui -perm -u+x | head -1)"
[ -n "$launcher" ] || die "the build finished but there is no stud-ui under $build_dir"

if [ "$with_tests" -eq 1 ]; then
    say "running the tests"
    ctest --test-dir "$build_dir" --output-on-failure
fi

# ---------------------------------------------------------------------------
# Where it is and how to start it.

# A path inside the checkout reads better relative to it; one chosen elsewhere stays whole.
show="$launcher"
case "$(readlink -f "$launcher")" in "$PWD"/*) show="./${launcher#"$PWD"/}" ;; esac

size="$(du -h "$launcher" | cut -f1)"
say "built $show ($size)"
cat >&2 <<EOF

  Run it:            $show
  Settings only:     $show --settings
  A second account:  $show --instance alt

  The first launch downloads the macOS client, a few hundred megabytes.
  This build runs against this machine's Qt and SDL. A taskbar entry and
  roblox:// links come from packaging/install-desktop-entry.sh, and a
  portable single file from packaging/build-appimage.sh.
EOF

if [ "$run_after" -eq 1 ]; then
    say "starting Stud"
    exec "$launcher"
fi
