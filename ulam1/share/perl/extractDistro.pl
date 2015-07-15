#!/usr/bin/perl -w
# -*- mode:perl -*-

my $MFM_TREE = shift @ARGV;
my $ULAM_TREE = shift @ARGV;
my $OUTPUT_DIR = shift @ARGV;

$MFM_TREE =~ s!/$!!;
$ULAM_TREE =~ s!/$!!;
$OUTPUT_DIR =~ s!/$!!;

defined $MFM_TREE and -d $MFM_TREE
    or die "Usage: $0 MFM_TREE ULAM_TREE OUTPUT_DIR (bad MFM_TREE)\n";
defined $ULAM_TREE and -d $ULAM_TREE
    or die "Usage: $0 MFM_TREE ULAM_TREE OUTPUT_DIR (bad ULAM_TREE)\n";
defined $OUTPUT_DIR and !-e $OUTPUT_DIR
    or die "Usage: $0 MFM_TREE ULAM_TREE OUTPUT_DIR (bad existing OUTPUT_DIR)\n";

my %categories = (
    "MFM_source" =>       ["MFM", "find src -name '*.cpp' -o -name '*.tmpl' -o -name '*.src'"],
    "MFM_headers" =>      ["MFM", "find src -name '*.inc' -o -name '*.h' -o -name '*.tcc'"],
    "MFM_makefiles" =>    ["MFM", "find Makefile *.mk config src -name '[Mm]akefile' -o -name '*.mk'"],
    "MFM_shared_files" => ["MFM", "find res"],
    "MFM_libraries" =>    ["MFM", "find build -name '*.a'"],
    "MFM_binaries" =>     ["MFM", "find bin -name 'mfms' -o -name 'mfzmake' -o -name 'mfzrun'"],

    "ULAM_source" =>      ["ULAM", "find src -name '*.cpp' -o -name '*.tmpl' -o -name '*.src'"],
    "ULAM_headers" =>     ["ULAM", "find include -name '*.h' -o  -name '*.inc'"],
    "ULAM_makefiles" =>   ["ULAM", "find Makefile *.mk src -name '[Mm]akefile' -o -name '*.mk'"],
    "ULAM_binaries" =>    ["ULAM", "find bin -name 'ulam' -o -name 'culam'"],
    "ULAM_shared_files"=> ["ULAM", "find share"],
    );

use File::Path qw(make_path remove_tree);

make_path( $OUTPUT_DIR );
my $DISTRO_MFM = `readlink -f $OUTPUT_DIR/MFM`;
my $DISTRO_ULAM = `readlink -f $OUTPUT_DIR/ULAM`;

chomp($DISTRO_MFM);
chomp($DISTRO_ULAM);
make_path( $DISTRO_MFM, $DISTRO_ULAM );

my %indirs = ("MFM" => $MFM_TREE, "ULAM" => $ULAM_TREE);
my %outdirs = ("MFM" => $DISTRO_MFM, "ULAM" => $DISTRO_ULAM);

for my $c (sort keys %categories) {
    my ($tree, $cmd) = @{$categories{$c}};
    my $indir = $indirs{$tree};
    my $outdir = $outdirs{$tree};
    my $lines = `cd $indir; $cmd`;
    die "'cd $indir; $cmd' found nothing -- are the tree dirs right?\n"
        if $lines eq "";
    my @lines = split("\n",$lines);
    for my $line (@lines) {
        my $src = "$indir/$line";
        die "Can't find '$src' -- are the tree dirs right?" unless -e "$src";
        next if -d $src;
        $src =~ s!^$indir/!! or die "Couldn't find '$indir' at front of '$src'\n";
        my $res = `mkdir -p $outdir && cd $indir;cp --parents $src $outdir && echo -n OK`;
        die "Copy $src => $outdir failed" unless $res eq "OK";
    }
}

################# THE TREE IS CREATED.  FINAL CUSTOMIZATIONS

`echo "MFM_ROOT_DIR := $DISTRO_MFM" > $DISTRO_ULAM/Makefile.local.mk`;
`mkdir $DISTRO_ULAM/build`;
