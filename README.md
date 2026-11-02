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
2. Provide a real WebView APK, see "WebView (known problem)" below.
3. From the root of your source tree run:

       bash /path/to/a2corelte-17.1-fixes/apply-fixes.sh

   or build directly:

       SRC=/path/to/lineage-17.1 bash /path/to/a2corelte-17.1-fixes/build-a2corelte.sh

## WebView (known problem)

The arm WebView prebuilt is not part of this repo and is currently not available from its usual place.

- In the LineageOS 17.1 manifest, `external/chromium-webview/prebuilt/arm` comes from `LineageOS/android_external_chromium-webview_prebuilt_arm`, branch `main`.
- On that branch `webview.apk` is a Git LFS pointer (a text file of about 133 bytes), not a real APK. It points to an object of 98,126,751 bytes.
- On 2 November 2026 the GitHub LFS server answered "Object does not exist on the server" for that object, so `repo init --git-lfs` or `git lfs pull` cannot fetch it either.
- Not tested: what the build does with the pointer file. Expect a build error or a ROM without a working WebView.

Workaround: put a real 32-bit arm WebView APK at `external/chromium-webview/prebuilt/arm/webview.apk` in your source tree before building. If you place it at `untracked/external/chromium-webview/prebuilt/arm/webview.apk` inside this repo, `apply-fixes.sh` copies it for you.

The working build used this file: Chromium WebView 111.0.5563.116, 89,767,144 bytes, sha256 `4bc506362baa4a3b22d5143b21d4eb86e7b1763882af6b555c2eaf5f048df340`. It is not hosted here and its origin is not documented. Other versions are untested. Only the arm prebuilt was checked, and LineageOS may have moved the file elsewhere (not verified).

## License

The scripts are under Apache-2.0. The patches apply to AOSP/LineageOS projects and remain under the licenses of those projects.
