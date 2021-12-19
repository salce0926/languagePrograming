#!/bin/bash
[ "x$1" = "x" ] && exit 1
#引数にファイルパスを代入
DIR=$1

for file in `\find $DIR -name '*.mpl'`; do
  ./parse $file
done