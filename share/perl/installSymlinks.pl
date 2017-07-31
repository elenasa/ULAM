#!/usr/bin/perl -w
# -*- mode:perl -*-
my $ULAM_LANGUAGE_VERSION = "ulam2";

my $ulamRootDir = shift @ARGV;
my $symlinkBinDir = shift @ARGV;
defined $ulamRootDir && defined $symlinkBinDir
    or die "Usage: $0 ULAM_ROOT_DIR SYMLINK_BIN_DIR";

my @ulam_programs = ("culam", "ulam");
my @mfm_programs = ("mfms", "mfzmake", "mfzrun");

checkName(\$ulamRootDir);

use File::Path qw(make_path);
make_path( $symlinkBinDir );
checkName(\$symlinkBinDir);

for my $u (@ulam_programs) {
    doLink("$ulamRootDir/$ULAM_LANGUAGE_VERSION/bin/$u", "$symlinkBinDir/$u");
}
for my $m (@mfm_programs) {
    doLink("$ulamRootDir/MFM/bin/$m", "$symlinkBinDir/$m");
}

sub doLink {
    my ($realProg, $linkName) = @_;
    die "No executable '$realProg'"
        unless -x $realProg;
    symlink $realProg,$linkName
        or die "Can't link $linkName=>$realProg: $!";
}

sub checkName {
    my $nref = shift;
    my $std = `readlink -f $$nref`;
    die "Not found '$$nref'" unless $std ne "";
    chomp($std);
    $$nref = $std;
}

exit 0;
