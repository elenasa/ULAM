#!/usr/bin/perl -w

BEGIN {
    use FindBin;
    push @INC, $FindBin::Bin;
}

my $version = "1.0";

# (Hopefully) Simple script to initialize a directory to be an
# 'ulam Project directory'
#
# Layout relative to ./ --
# .mfmproject           -- project settings file
# .gen/                 -- default ulam work dir
# lib/                  -- latest .so's preserved from work dir
# doc/
# src/
