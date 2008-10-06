#!/bin/bash

find /ardi/executor/src -type f -name '*.[chm]' -print | xargs co -l -f -q
find /ardi/executor/src -type f -name '*.[chm]' -print | xargs /ardi/executor/util/uncruft.pl
find /ardi/executor/src -type f -name '*.[chm]' -exec /ardi/executor/util/temp-hack.sh /ardi/executor/util/uncopyright.pl '{}' ';'
find /ardi/executor/src -type f -name '*.[cm]' -exec /ardi/executor/util/temp-hack.sh /ardi/executor/util/commonify.pl '{}' ';'
