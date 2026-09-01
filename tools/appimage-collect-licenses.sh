#!/usr/bin/env bash
# Collect licence and copyright information for everything an AppImage bundles.
#
#   usage: tools/appimage-collect-licenses.sh <AppDir>
#
# A .deb declares only what it ships, because its dependencies remain separate
# packages carrying their own copyright files. An AppImage carries those
# libraries inside it, so their notices have to travel with it -- and most of
# them here are LGPL (GTK, glib, pango and friends), where the obligation also
# covers saying where the corresponding source can be had.
#
# The mapping is mechanical rather than researched: every bundled library came
# from a Debian/Ubuntu package on the build host, so dpkg can name it, and the
# archive already holds a copyright file for each. Run this after linuxdeploy
# has populated the AppDir and before it packages the image.
set -euo pipefail

APPDIR="${1:?AppDir path}"
DOCDIR="$APPDIR/usr/share/doc"
MANIFEST="$DOCDIR/BUNDLED-LIBRARIES.txt"

mkdir -p "$DOCDIR"

{
  echo "Libraries bundled in this AppImage"
  echo "=================================="
  echo
  echo "Each library below was taken from the Debian/Ubuntu package named"
  echo "beside it on the build host. That package's copyright file is included"
  echo "under usr/share/doc/<package>/copyright in this image."
  echo
  echo "The corresponding source for any of them can be obtained with:"
  echo "    apt-get source <package>=<version>"
  echo "from the distribution and release recorded below."
  echo
  echo "Build host: $( (. /etc/os-release && echo "$PRETTY_NAME") 2>/dev/null || echo unknown)"
  echo
  printf '%-44s %-34s %s\n' "LIBRARY" "PACKAGE" "VERSION"
} > "$MANIFEST"

found=0
unowned=0

while IFS= read -r so; do
  base="$(basename "$so")"
  pkg=""
  # Try the usual multiarch location first, then fall back to a name search.
  for candidate in "/usr/lib/x86_64-linux-gnu/$base" "/lib/x86_64-linux-gnu/$base" "/usr/lib/$base"; do
    pkg="$(dpkg -S "$candidate" 2>/dev/null | head -1 | cut -d: -f1 || true)"
    [ -n "$pkg" ] && break
  done
  if [ -z "$pkg" ]; then
    pkg="$(dpkg -S "$base" 2>/dev/null | head -1 | cut -d: -f1 || true)"
  fi

  if [ -n "$pkg" ]; then
    ver="$(dpkg-query -W -f='${Version}' "$pkg" 2>/dev/null || echo unknown)"
    printf '%-44s %-34s %s\n' "$base" "$pkg" "$ver" >> "$MANIFEST"
    if [ -f "/usr/share/doc/$pkg/copyright" ] && [ ! -f "$DOCDIR/$pkg/copyright" ]; then
      install -Dm644 "/usr/share/doc/$pkg/copyright" "$DOCDIR/$pkg/copyright"
    fi
    found=$((found + 1))
  else
    # Built by this job rather than installed from a package -- its licence is
    # collected separately, from its own source tree.
    printf '%-44s %-34s %s\n' "$base" "(built from source in this job)" "-" >> "$MANIFEST"
    unowned=$((unowned + 1))
  fi
done < <(find "$APPDIR" -name '*.so*' -type f | sort)

echo >> "$MANIFEST"
echo "$found libraries from distribution packages, $unowned built in this job." >> "$MANIFEST"

echo "collected copyright for $found bundled libraries ($unowned built here)"
echo "manifest: $MANIFEST"
