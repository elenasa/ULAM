#!/usr/bin/perl -w
# -*- mode:perl -*-

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
    or die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (bad src|bin)\n";

my $PKGNAME_ROOT_DIR = shift @ARGV;
$PKGNAME_ROOT_DIR =~ s!/$!!;
defined $PKGNAME_ROOT_DIR and -d $PKGNAME_ROOT_DIR
    or die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (bad PKGNAME_ROOT_DIR)\n";

my $OUTPUT_DIR = shift @ARGV;
defined $OUTPUT_DIR
    or die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (missing OUTPUT_DIR)\n";
$OUTPUT_DIR =~ s!/$!!;
-e $OUTPUT_DIR && !-d $OUTPUT_DIR
    and die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (not a dir OUTPUT_DIR)\n";

$OUTPUT_DIR =~ s!/$!!;
# Last component of OUTPUT_DIR must be the package name
$OUTPUT_DIR =~ m!.*?([^/]+)$! or die "No pkg name match in '$OUTPUT_DIR'";
my $PACKAGE_NAME = $1;
$PACKAGE_NAME =~ /^[a-zA-Z][a-zA-Z0-9]*$/
    or die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (bad format PACKAGE_NAME at end of OUTPUT_DIR)\n";
my $MAGIC_PACKAGE_VERSION = "";
if ($PACKAGE_NAME =~ /.*?([0-9]+)$/) {
    $MAGIC_PACKAGE_VERSION = $1;
}
print "Package name '$PACKAGE_NAME', Magic package version '$MAGIC_PACKAGE_VERSION'\n";


if (scalar(@ARGV) != 0) {
    die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (extra arguments: @ARGV)\n";
}

# The MFM tree shall be at $PKGNAME_ROOT_DIR/MFM
# The ULAM tree shall be at $PKGNAME_ROOT_DIR/ULAM

my $MFM_TREE = "$PKGNAME_ROOT_DIR/MFM";
my $ULAM_TREE = "$PKGNAME_ROOT_DIR/ULAM";

defined $MFM_TREE and -d $MFM_TREE
    or die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (not found '$MFM_TREE')\n";
defined $ULAM_TREE and -d $ULAM_TREE
    or die "Usage: $0 src|bin PKGNAME_ROOT_DIR OUTPUT_DIR (not found '$ULAM_TREE')\n";

my %categories = (
    "MFM_source" =>       ["MFM", "src", "find src -name '*.cpp' -o -name '*.tmpl' -o -name '*.src'"],
    "MFM_headers" =>      ["MFM", "src", "find src -name '*.inc' -o -name '*.h' -o -name '*.tcc'"],
    "MFM_makefiles" =>    ["MFM", "src", "find Makefile *.mk config src -name '[Mm]akefile' -o -name '*.mk'"],
    "MFM_shared_files" => ["MFM", "src", "find res"],
    "MFM_libraries" =>    ["MFM", "bin", "find build -name '*.a'"],
    "MFM_binaries" =>     ["MFM", "bin", "find bin -name 'mfms' -o -name 'mfzmake' -o -name 'mfzrun'"],

    "MFM_topmakefile" =>  ["MFM", "usrc", "ls -1 Makefile"],
    "MFM_doc" =>          ["MFM", "usrc", "ls -1 *.md"],

    "ULAM_source" =>      ["ULAM", "usrc", "find src -name '*.cpp' -o -name '*.tmpl' -o -name '*.src'"],
    "ULAM_headers" =>     ["ULAM", "usrc", "find include -name '*.h' -o  -name '*.inc'"],
    "ULAM_makefiles" =>   ["ULAM", "usrc", "find Makefile *.mk src -name '[Mm]akefile' -o -name '*.mk'"],
    "ULAM_binaries" =>    ["ULAM", "bin", "find bin -name 'ulam' -o -name 'culam'"],
    "ULAM_shared_files"=> ["ULAM", "src", "find share"],

    "ULAM_packaging" =>   ["ULAM", "usrc", "find debian"],
    "ULAM_topmakefile" => ["ULAM", "usrc", "ls -1 Makefile"],
    "ULAM_doc" =>         ["ULAM", "usrc", "ls -1 *.md"],
    );

use File::Path qw(make_path remove_tree);

make_path( $OUTPUT_DIR );
my $DISTRO_MFM = `readlink -f $OUTPUT_DIR/MFM`;
my $DISTRO_ULAM = `readlink -f $OUTPUT_DIR/ULAM`;
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
    die "For $c/$type: 'cd $indir; $cmd' found nothing -- are the tree dirs right?\n"
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

# Copy up the whole debian dir
`cp -a $OUTPUT_DIR/ULAM/debian/ $OUTPUT_DIR/debian`;
my @files = <$OUTPUT_DIR/debian/*>;

# Replace .tmpl files
foreach my $file (@files) {
    next if $file =~ /^[.]/;
    next if -d $file;
    my $ifile = $file;
    next unless ($file =~ s/[.]tmpl$//);
    open(RF,"<$ifile") or die "$!";
    my $string;
    {
        local $/=undef;
        $string = <RF>;
        close RF or die "$!";
    }
    $string =~ s/<XD:PKG_NAME>/$PACKAGE_NAME/g;
    $string =~ s/<XD:MAGIC_PKG_VERSION>/$MAGIC_PACKAGE_VERSION/g;
    $string =~ s/<XD:COMMENT[^>]*>//sg;

    open(WF,">$file") or die "$!";
    print WF $string;
    close(WF) or die "$!";

    unlink($ifile) or die "$!";
}

open(TOPMAKE,">$OUTPUT_DIR/Makefile") or die "$!";
print TOPMAKE <<EOM;
all:	FORCE
	make -C MFM
	make -C ULAM

install:	FORCE
	make -f debian/Makedebian.mk install

.PHONY:	FORCE
EOM
close TOPMAKE or die $!;

mkdir("$DISTRO_ULAM/build") or die "$!";

exit 0;
