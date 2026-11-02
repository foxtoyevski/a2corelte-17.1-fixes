#!/usr/bin/env bash
#
# a2corelte / universal7870 LineageOS 17.1 - yerel AOSP fixlerini geri yukler.
#
# Bu fixlerin COGUNLUGU device.mk'da DEGIL, dogrudan AOSP/framework kaynaginda
# commit edilmemis degisikliklerdir (build/make, frameworks/av, hardware/
# interfaces/audio, system/core, system/netd, hardware/ril, sepolicy, ...).
# `repo sync` veya `git checkout -- .` bunlari siler ve ROM boot etmez.
#
# Kullanim (AOSP/Lineage agacinin KOKUNDEN):
#     bash /path/to/a2corelte-17.1-fixes/apply-fixes.sh
#
# Idempotenttir: zaten uygulanmis degisiklikleri atlar, guvenle tekrar calisir.
#
set -uo pipefail

BACKUP_DIR="$(cd "$(dirname "$0")" && pwd)"
TOP="${ANDROID_BUILD_TOP:-$(pwd)}"

echo "Kaynak kok: $TOP"
echo "Yedek     : $BACKUP_DIR"

if [ ! -d "$TOP/build/make" ] || [ ! -d "$TOP/frameworks" ]; then
    echo "HATA: '$TOP' bir AOSP/Lineage kaynak agaci gibi gorunmuyor."
    echo "      Script'i agacin kokunden calistir."
    exit 1
fi

applied=0; already=0; failed=0

# 1) tracked degisiklikler
while IFS= read -r proj; do
    [ -z "$proj" ] && continue
    p="$BACKUP_DIR/patches/${proj//\//__}.patch"
    [ -s "$p" ] || continue
    if git -C "$TOP/$proj" apply --reverse --check "$p" 2>/dev/null; then
        echo "  = zaten uygulanmis: $proj"
        already=$((already+1))
        continue
    fi
    if git -C "$TOP/$proj" apply --whitespace=nowarn "$p" 2>/dev/null; then
        echo "  + uygulandi: $proj"
        applied=$((applied+1))
    else
        echo "  ! UYGULANAMADI (elle bak): $proj"
        failed=$((failed+1))
    fi
done < "$BACKUP_DIR/modified-projects.txt"

# 2) yeni (untracked) ve binary dosyalar
if [ -d "$BACKUP_DIR/untracked" ]; then
    ( cd "$BACKUP_DIR/untracked" && find . -type f -print0 ) | while IFS= read -r -d '' f; do
        dest="$TOP/${f#./}"
        mkdir -p "$(dirname "$dest")"
        if ! cmp -s "$BACKUP_DIR/untracked/${f#./}" "$dest" 2>/dev/null; then
            cp -f "$BACKUP_DIR/untracked/${f#./}" "$dest"
            echo "  + dosya: ${f#./}"
        fi
    done
fi

echo
echo "Ozet: uygulandi=$applied, zaten_vardi=$already, hata=$failed"
if [ "$failed" -gt 0 ]; then
    echo "DIKKAT: bazi yamalar uygulanamadi. 'git apply --reject' ile tek tek bak."
    exit 2
fi
echo "Kontrol icin:  cd $TOP && repo status"
