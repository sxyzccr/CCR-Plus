#!/bin/bash

if [ x$1 != x ]
then
  appDir=$1
else
  appDir=$PWD
fi

if [ x"$2" != x ]
then
  linkDir=$2
else
  linkDir=$PWD
fi

version="v1.1.0"
filename=$linkDir/ccr-plus.desktop

ln -sf lib/libicudata.so.56.1 lib/libicudata.so.56
ln -sf lib/libicui18n.so.56.1 lib/libicui18n.so.56
ln -sf lib/libicuuc.so.56.1 lib/libicuuc.so.56
ln -sf lib/libQt5Core.so.5.8.0 lib/libQt5Core.so.5
ln -sf lib/libQt5DBus.so.5.8.0 lib/libQt5DBus.so.5
ln -sf lib/libQt5Gui.so.5.8.0 lib/libQt5Gui.so.5
ln -sf lib/libQt5Widgets.so.5.8.0 lib/libQt5Widgets.so.5
ln -sf lib/libQt5XcbQpa.so.5.8.0 lib/libQt5XcbQpa.so.5
ln -sf lib/libQt5Xml.so.5.8.0 lib/libQt5Xml.so.5

echo "[Desktop Entry]" > "$filename"
echo "Version="$version >> "$filename"
echo "Type=Application" >> "$filename"
echo "Terminal=false" >> "$filename"
echo "Name=CCR Plus" >> "$filename"
echo "Comment[zh_CN]=CCR Plus 测评器" >> "$filename"
echo "GenericName[zh_CN]=CCR Plus 测评器" >> "$filename"
echo Path=$appDir >> "$filename"
echo Exec=\"$appDir/run.sh\" >> "$filename"
echo Icon=$appDir/icon/CCR.ico >> "$filename"
echo "Categories=Qt;Development" >> "$filename"
chmod +x "$filename"

