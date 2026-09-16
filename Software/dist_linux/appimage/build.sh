#!/bin/sh -xe

export QT_SELECT=qt5

cd ../..
qmake -r
make
cd -

VERSION=`cat ../../RELEASE_VERSION`
ARCH=${1:-x86_64}
APPDIR=AppDir
OUTFILE="prismatik_${VERSION}_${ARCH}.AppImage"

rm -rf "$APPDIR" "$OUTFILE" Prismatik-*.AppImage
mkdir -p .tools

install -Dm755 -s ../../bin/Prismatik "$APPDIR/usr/bin/prismatik"
install -Dm644 ../package_template/usr/share/applications/prismatik.desktop "$APPDIR/prismatik.desktop"
install -Dm644 ../package_template/usr/share/pixmaps/Prismatik.png "$APPDIR/Prismatik.png"

sed -i 's#^Icon=.*#Icon=Prismatik#' "$APPDIR/prismatik.desktop"

curl -fsSL -o .tools/linuxdeploy.AppImage \
	https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
curl -fsSL -o .tools/linuxdeploy-plugin-qt.AppImage \
	https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

chmod +x .tools/linuxdeploy.AppImage .tools/linuxdeploy-plugin-qt.AppImage

export ARCH
export APPIMAGE_EXTRACT_AND_RUN=1

.tools/linuxdeploy.AppImage \
	--appdir "$APPDIR" \
	-d "$APPDIR/prismatik.desktop" \
	-i "$APPDIR/Prismatik.png" \
	--plugin qt \
	--output appimage

generated_appimage=`find . -maxdepth 1 -type f -name 'Prismatik-*.AppImage' | head -n1`
[ -z "$generated_appimage" ] && echo "Failed to generate AppImage" && exit 1
mv "$generated_appimage" "$OUTFILE"
