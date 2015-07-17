#!/usr/bin/perl -w
# -*- mode:perl -*-

my $ULAM_VERSION_SUBTREE = "ulam1";

# $content eq "src" means:
#
#   Everything needed to compile culam itself, and mfm.  Used to
#   extract from the combined git repos what will become everything in
#   the debian source package.
#
# CONTENT eq "bin" means:
#
#   Everything needed to use included culam (etc) binaries to compile
#   .ulam files, but not the sources of culam itself.  (Same as "src"
#   but excludes files tagged "usrc" below).  Used to extract what
#   will become the debian binary package, and will ultimately get
#   installed under /usr/lib/ulam
#
my $content = shift @ARGV;
defined $content and ($content eq "src" or $content eq "bin")
    or die "Usage: $0 src|bin COMBINED_ROOT_DIR OUTPUT_DIR (bad src|bin)\n";

my $COMBINED_ROOT_DIR = shift @ARGV;
defined $COMBINED_ROOT_DIR and -d $COMBINED_ROOT_DIR
    or die "Usage: $0 src|bin COMBINED_ROOT_DIR OUTPUT_DIR (bad MFM_TREE)\n";
$COMBINED_ROOT_DIR =~ s!/$!!;

my $OUTPUT_DIR = shift @ARGV;
defined $OUTPUT_DIR
    or die "Usage: $0 src|bin COMBINED_ROOT_DIR OUTPUT_DIR (missing OUTPUT_DIR)\n";
$OUTPUT_DIR =~ s!/$!!;
-e $OUTPUT_DIR && !-d $OUTPUT_DIR
    and die "Usage: $0 src|bin COMBINED_ROOT_DIR OUTPUT_DIR (not a dir OUTPUT_DIR)\n";

# The MFM tree shall be at $COMBINED_ROOT_DIR/MFM
# The ULAM tree shall be at $COMBINED_ROOT_DIR/$ULAM_VERSION_SUBTREE

my $MFM_TREE = "$COMBINED_ROOT_DIR/MFM";
my $ULAM_TREE = "$COMBINED_ROOT_DIR/$ULAM_VERSION_SUBTREE";

defined $MFM_TREE and -d $MFM_TREE
    or die "Usage: $0 src|bin COMBINED_ROOT_DIR OUTPUT_DIR (not found '$MFM_TREE')\n";
defined $ULAM_TREE and -d $ULAM_TREE
    or die "Usage: $0 src|bin COMBINED_ROOT_DIR OUTPUT_DIR (not found '$ULAM_TREE')\n";

my %categories = (
    "MFM_source" =>       ["MFM", "src", "find src -name '*.cpp' -o -name '*.tmpl' -o -name '*.src'"],
    "MFM_headers" =>      ["MFM", "src", "find src -name '*.inc' -o -name '*.h' -o -name '*.tcc'"],
    "MFM_makefiles" =>    ["MFM", "src", "find Makefile *.mk config src -name '[Mm]akefile' -o -name '*.mk'"],
    "MFM_shared_files" => ["MFM", "src", "find res"],
    "MFM_libraries" =>    ["MFM", "bin", "find build -name '*.a'"],
    "MFM_binaries" =>     ["MFM", "bin", "find bin -name 'mfms' -o -name 'mfzmake' -o -name 'mfzrun'"],

    "ULAM_source" =>      ["ULAM", "usrc", "find src -name '*.cpp' -o -name '*.tmpl' -o -name '*.src'"],
    "ULAM_headers" =>     ["ULAM", "usrc", "find include -name '*.h' -o  -name '*.inc'"],
    "ULAM_makefiles" =>   ["ULAM", "usrc", "find Makefile *.mk src -name '[Mm]akefile' -o -name '*.mk'"],
    "ULAM_binaries" =>    ["ULAM", "bin", "find bin -name 'ulam' -o -name 'culam'"],
    "ULAM_shared_files"=> ["ULAM", "src", "find share"],

    "ULAM_packaging" =>   ["TOP", "usrc", "find debian"],
    "ULAM_topmakefile" => ["TOP", "usrc", "ls -1 Makefile"],
    "ULAM_doc" =>         ["TOP", "usrc", "ls -1 *.md"],
    );

use File::Path qw(make_path remove_tree);

make_path( $OUTPUT_DIR );
my $DISTRO_MFM = `readlink -f $OUTPUT_DIR/MFM`;
my $DISTRO_ULAM = `readlink -f $OUTPUT_DIR/$ULAM_VERSION_SUBTREE`;
my $DISTRO_TOP = `readlink -f $OUTPUT_DIR`;

chomp($DISTRO_MFM);
chomp($DISTRO_ULAM);
chomp($DISTRO_TOP);
make_path( $DISTRO_MFM, $DISTRO_ULAM );

my %indirs = ("MFM" => $MFM_TREE, "ULAM" => $ULAM_TREE, "TOP" => "$ULAM_TREE/.." );
my %outdirs = ("MFM" => $DISTRO_MFM, "ULAM" => $DISTRO_ULAM, "TOP" => $DISTRO_TOP );

for my $c (sort keys %categories) {
    my ($tree, $type, $cmd) = @{$categories{$c}};
    next if $content eq "bin" and $type eq "usrc";

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
        my $cmd = "mkdir -p $outdir && cd $indir;cp --parents $src $outdir";
        print " $cmd..";
        my $res = `$cmd || echo -n \$?`;
        if ($res eq "") {
            print "OK\n"
        } else {
            die "FAILED ($res)\n";
        }
    }
}

################# THE TREE IS CREATED.  FINAL CUSTOMIZATIONS

`echo "MFM_ROOT_DIR := $DISTRO_MFM" > $DISTRO_ULAM/Makefile.local.mk`
    if $content ne "bin";
`mkdir $DISTRO_ULAM/build`;
exit 0;
