# a2corelte-17.1-fixes

Source-level fixes for building LineageOS 17.1 (Android 10) for the Samsung Galaxy A2 Core (SM-A260F, a2corelte).

Most boot-critical fixes are not in the device tree. They are changes to AOSP/Lineage projects (`build/make`, `frameworks/av`, `hardware/interfaces`, `system/core`, ...). `repo sync` or `git checkout -- .` removes them, so they are kept here as patches.

## Status

Extracted from a working LineageOS 17.1 build (October 2026).
Not re-verified from a clean tree. If a patch fails to apply or the
build breaks, please open an issue.

## Contents

- `patches/`: one `git diff` patch per modified project
- `modified-projects.txt`: the projects the patches apply to
- `untracked/`: new files that are not part of any upstream project, copied into the source tree as-is
- `apply-fixes.sh`: applies everything, safe to run repeatedly
- `build-a2corelte.sh`: build helper (applies the fixes, then runs lunch and mka bacon)

## Usage

1. Sync LineageOS 17.1 and add the device trees, see [android_device_samsung_a2corelte](https://github.com/foxtoyevski/android_device_samsung_a2corelte).
2. Download `webview.apk` from the Releases page of this repo and put it at `untracked/external/chromium-webview/prebuilt/arm/webview.apk` (see WebView below).
3. From the root of your source tree run:

       bash /path/to/a2corelte-17.1-fixes/apply-fixes.sh

   or build directly:

       SRC=/path/to/lineage-17.1 bash /path/to/a2corelte-17.1-fixes/build-a2corelte.sh

## WebView

In the source tree, `external/chromium-webview/prebuilt/arm/webview.apk` is a Git LFS pointer file, not a real APK. Replace it with the APK from the Releases page (89,767,144 bytes, sha256 `4bc506362baa4a3b22d5143b21d4eb86e7b1763882af6b555c2eaf5f048df340`). `apply-fixes.sh` copies it into the tree if it is placed under `untracked/` as described above.

## License

The scripts are under Apache-2.0. The patches apply to AOSP/LineageOS projects and remain under the licenses of those projects.
