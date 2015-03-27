#!/bin/bash

if [ "$#" -lt 3 ] || [ ! -f "$1" ]; then
    echo "Needs at least three parameters:"
    echo "- An input image file"
    echo "- A color count to quantize the image to (256 or less)"
    echo "- An output image file"
    echo "- (Optional) target palette bit depth (1 - 8)"
    echo "- (Optional) turn on dithering ('d')"
    exit 1
fi
convert $1 -alpha opaque -print "%w %h;" $1.rgb >$1.tmp
read -d ";" -s wd hg <$1.tmp
rm $1.tmp
./insaniquant $1.rgb ${wd} ${hg} $2 $3.rgb $4 $5
rm $1.rgb
convert -colors $2 -depth 8 -size ${wd}x${hg} rgb:$3.rgb $3
rm $3.rgb
convert $3 $1 -alpha set -compose copy-opacity -composite $3
pngcrush $3 $3.tmp
rm $3
mv $3.tmp $3
