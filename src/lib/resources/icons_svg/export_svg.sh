#!/bin/bash
svg_dir=$1
png_dir=$2
size=$3
for d in $svg_dir ; do
        for f in `find $d -type f -name "*.svg"` ; do
                filename=$(basename $f)
                extension="${filename##*.}"
                filename="${filename%.*}"
                
                echo "Now Processing File: ${filename}"
                inkscape -z -e "$png_dir/$filename".png -w $3 -h $3 "$f"
                inkscape -z -e "$png_dir/$filename"@2x.png -w $((2*$size)) -h  $((2*$size)) "$f"
        done
done
