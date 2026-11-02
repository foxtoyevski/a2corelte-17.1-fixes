#!/usr/bin/env bash
#
# Samsung Galaxy A2 Core (SM-A260F) - LineageOS 17.1 build helper
#
# Usage:
#     SRC=/path/to/lineage-17.1 bash build-a2corelte.sh          # incremental build
#     SRC=/path/to/lineage-17.1 bash build-a2corelte.sh clean    # clobber first
#
# Optional environment variables:
#     JAVA_HOME  path to OpenJDK 8 (recommended for LineageOS 17.1)
#     JOBS       parallel jobs (default: number of CPU cores)
#
set -o pipefail

FIXDIR="$(cd "$(dirname "$0")" && pwd)"
SRC="${SRC:?Set SRC to the root of your LineageOS 17.1 source tree}"
JOBS="${JOBS:-$(nproc)}"

if [ -n "${JAVA_HOME:-}" ]; then
    export JAVA_HOME
    export PATH="$JAVA_HOME/bin:$PATH"
fi

cd "$SRC" || { echo "source tree not found: $SRC"; exit 1; }

echo "== 1) Apply framework fixes =="
ANDROID_BUILD_TOP="$SRC" bash "$FIXDIR/apply-fixes.sh" || exit 2

echo
echo "== 2) Environment =="
java -version 2>&1 | head -1
source build/envsetup.sh

echo
echo "== 3) lunch lineage_a2corelte-userdebug =="
lunch lineage_a2corelte-userdebug || exit 3

if [ "${1:-}" = "clean" ]; then
    echo
    echo "== 3b) clobber =="
    mka clobber
fi

echo
echo "== 4) Building (-j${JOBS}) =="
echo "   Output: $SRC/out/target/product/a2corelte/lineage-17.1-*-a2corelte.zip"
LOG="$SRC/out/build-a2corelte-$(date +%Y%m%d-%H%M%S).log"
mka bacon -j"$JOBS" 2>&1 | tee "$LOG"
rc=${PIPESTATUS[0]}

echo
echo "== 5) Result =="
ls -la "$SRC"/out/target/product/a2corelte/lineage-17.1-*-a2corelte.zip 2>/dev/null
echo "log: $LOG"
exit "$rc"
