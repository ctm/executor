#!/usr/bin/perl -ni

s/blockinterrupts_t/virtual_int_state_t/g;
s/unblockinterrupts\s*\(\&([\w\d]+)\)/restore_virtual_ints ($1)/g;
s/blockinterrupts\s*\(\&([\w\d]+)\)/$1 = block_virtual_ints ()/g;

print;
