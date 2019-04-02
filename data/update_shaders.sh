#!/bin/sh

cd "`dirname "$0"`"

glslangValidator -V draw.vert
mv vert.spv draw_vert.spv

glslangValidator -V draw.frag
mv frag.spv draw_frag.spv
