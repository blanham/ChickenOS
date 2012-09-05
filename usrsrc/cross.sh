#!/bin/bash
export HOST=i386-pc-chickenos
directory=$(dirname -- $(readlink -fn -- "$0"))

export PREFIX=$directory"/local"
#echo $PREFIX
export PATH=$PATH:$PREFIX/bin
