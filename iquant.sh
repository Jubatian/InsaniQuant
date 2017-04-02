#!/bin/bash

if [ "$#" -lt 4 ] || [ ! -f "$1" ]; then
    echo "Needs at least four parameters:"
    echo "- An input image file"
    echo "- A color count to quantize the image to (256 or less)"
    echo "- An output image file"
    echo "- An output format"
    echo "- (Optional) turn on hexadecimal output ('h')"
    echo "- (Optional) turn on dithering ('d')"
    exit 1
fi
convert $1 -alpha opaque -print "%w %h;" $1.rgb >$1.tmp
read -d ";" -s wd hg <$1.tmp
rm $1.tmp
./insaniquant $1.rgb ${wd} ${hg} $2 $3 $4 $5 $6
rm $1.rgb
