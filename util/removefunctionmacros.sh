#!/bin/bash
# single-use script used once for getting rid of the old P1..P11 macros
# see git commit 6ba7fb2348287ac8b265706d50732008c9d3c185

for f in *.cpp; do
    m4 -P ../util/removefunctionmacros.m4 $f | sed '/./,$!d' > tmp.cpp
    mv tmp.cpp $f
done