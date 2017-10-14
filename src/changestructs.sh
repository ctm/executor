#!/bin/sh

for x in `find . -name \*.h`; do
    ruby changestructs.rb < $x > tmp
    mv tmp $x
done

