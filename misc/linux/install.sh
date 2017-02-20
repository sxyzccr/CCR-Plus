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

ln -sf libicudata.so.56.1     "$appDir/lib/libicudata.so.56"
ln -sf libicui18n.so.56.1     "$appDir/lib/libicui18n.so.56"
ln -sf libicuuc.so.56.1       "$appDir/lib/libicuuc.so.56"
ln -sf libQt5Core.so.5.8.0    "$appDir/lib/libQt5Core.so.5"
ln -sf libQt5DBus.so.5.8.0    "$appDir/lib/libQt5DBus.so.5"
ln -sf libQt5Gui.so.5.8.0     "$appDir/lib/libQt5Gui.so.5"
ln -sf libQt5Widgets.so.5.8.0 "$appDir/lib/libQt5Widgets.so.5"
ln -sf libQt5XcbQpa.so.5.8.0  "$appDir/lib/libQt5XcbQpa.so.5"
ln -sf libQt5Xml.so.5.8.0     "$appDir/lib/libQt5Xml.so.5"

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

