#!/bin/bash

if [ "$#" -lt 2 ] || [ ! -d "$1" ]; then
    echo "Needs at least two parameters:"
    echo "- A directory to process png images within"
    echo "- A color count to quantize every image to (256 or less)"
    echo "- (Optional) target palette bit depth (1 - 8)"
    echo "- (Optional) turn on dithering ('d')"
    exit 1
fi

for i in ${1}/*.png; do
    convert ${i} -alpha opaque -print "%w %h;" ${i}.rgb >${i}.tmp
    read -d ";" -s wd hg <${i}.tmp
    rm ${i}.tmp
    ./insaniquant ${i}.rgb ${wd} ${hg} ${2} ${i}.rgb.tmp ${3} ${4}
    rm ${i}.rgb
    mv ${i}.rgb.tmp ${i}.rgb
    convert +dither -colors ${2} -depth 8 -size ${wd}x${hg} rgb:${i}.rgb ${i}.t.png
    rm ${i}.rgb
    convert ${i}.t.png ${i} -alpha set -compose copy-opacity -composite ${i}.t.png
    pngcrush ${i}.t.png ${i}
    rm ${i}.t.png
done
