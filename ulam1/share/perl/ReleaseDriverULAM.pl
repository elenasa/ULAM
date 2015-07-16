#!/usr/bin/perl -w
my %steps; sub step { $steps{scalar(keys %steps).". ".$_[0]} = $_[1]; }
#
# This script runs as the main event of the ULAM_DISTRO_CREATION_TIME
# era.  It does the following steps:
#
step("REPO_CHECK_OUT",<<EOS);

  Checks out the MFM and ULAM repos, with ulam1 and MFM as siblings.

EOS
step("REPO_BUILD",<<EOS);

  Builds both repos, to ensure they do build at _REPO_BUILD_TIME eras,
  to get final_TREEVERSION.mk files created, and to get the base ulam
  version number that we are releasing.

EOS
step("FIRST_EXTRACT",<<EOS);

  Runs extractDistro.pl on those built repos, targeting a temporary
  directory, to get just the distro files.

EOS
step("TREE_BUILD",<<EOS);

  Builds in that tempdir, to ensure both contained trees do build (as
  an incomplete simulation of the ULAM_DISTRO_BUILD_TIME era).
  Assuming all goes well, then discards that tempdir.

EOS
step("SECOND_EXTRACT",<<EOS);

  Runs extractDistro.pl on the built repos again to a directory named
  for the ulam version we are exporting.  Then adds the canned debian
  directory, and creates the distro tar file from the resulting tree
  (withOUT having built in it).

EOS
step("DISTRO_BUILD",<<EOS);

  Untars the created tar file to yet another temporary directory, and
  builds it

EOS

## PRELIMINARIES TO ALL STEPS

$| = 1;
my $GIT_URL = "https://github.com/DaveAckley/ULAM.git";
my $MFM_GIT_URL = "https://github.com/DaveAckley/MFM.git";
my @DISTROS = ("precise", "trusty");

use Cwd 'abs_path';
use File::Basename;
use File::Temp qw/ tempfile tempdir /;

my $MY_PATH = abs_path($0);
my $MY_DIR = dirname($MY_PATH);

my $STATE_DIR = `readlink -f ~/pbuilder/localState`;
chomp($STATE_DIR);

my $mfm_version_tag = undef; # Set in step REPO_BUILD
my $ulam_version_tag = undef; # Set in step REPO_BUILD

my $workBaseDir = tempdir( CLEANUP => 0 );
my $workDir = "$workBaseDir/work";
mkdir($workDir) || die "$!";
chdir($workDir) || die "$!";

print "Work dir is: $workDir\n";

for my $step (sort keys %steps) {
    $step =~ /^(\d+)[.] (\w+)$/ or die;
    my ($num,$tag) = ($1,$2);
    print "\n-------------\nStarting step $step\n$steps{$step}";
    my $ret = &$tag();
    if ($ret ne "") {
        die "Step $tag failed: $ret\n";
    }
}

print "FINISHED: $workDir\n";

# Return home to let temp dir die
chdir || die "$!";

exit 0;

sub REPO_CHECK_OUT {
    print "Cloning repo $GIT_URL..";
    my $git_clone_output = `git clone $GIT_URL 2>&1`;
    print "done\n";

    print "Cloning repo $MFM_GIT_URL into ULAM..";
    my $git_mfm_clone_output = `cd ULAM;git clone $MFM_GIT_URL 2>&1`;
    print "done\n";

    print "Making Makefile.local.mk..";
    `echo 'MFM_ROOT_DIR := \$(ULAM_ROOT_DIR)/../MFM' > ULAM/ulam1/Makefile.local.mk`;
    print "done\n";
    return "";
}

sub REPO_BUILD {
    print "Building repos..";
    mkdir "logs" or die "mkdir: $!";
    my $ret;
    $ret = `cd ULAM && make >../logs/REPO_BUILD.log 2>&1 || echo -n \$?`;
    return "Repo build failed ($ret)"
        unless $ret eq "";
    print "OK\n";

    print "Getting version number from MFM..";
    $mfm_version_tag = `cd ULAM/MFM;make version`;
    chomp($mfm_version_tag);
    $mfm_version_tag =~ /^\d+[.]\d+[.]\d+$/ or return "MFM version not found, got '$mfm_version_tag'";
    print "$mfm_version_tag\n";

    print "Getting version number from ULAM..";
    $ulam_version_tag = `cd ULAM;make -C ulam1 -f Makedebian.mk --quiet version`;
    chomp($ulam_version_tag);
    $ulam_version_tag =~ /^\d+[.]\d+[.]\d+$/ or return "Ulam version not found, got '$ulam_version_tag'";
    print "$ulam_version_tag\n";

    print "Getting git version tags\n";
    my $f;
    $f = "ULAM/ulam1/ULAM_TREEVERSION.mk";
    if (-r $f) { print " ".`cat $f`; } else { return "Not found '$f'"; }
    $f = "ULAM/MFM/MFM_TREEVERSION.mk";
    if (-r $f) { print " ".`cat $f`; } else { return "Not found '$f'"; }

    return "";
}

sub FIRST_EXTRACT {
    print "Extracting files for test build..";
    my $extractPath = "ULAM/ulam1/share/perl/extractDistro.pl";
    my $ret = `$extractPath src ULAM extract1 >logs/FIRST_EXTRACT.log 2>&1 || echo \$?`;
    return "First extract failed ($ret)"
        unless $ret eq "";

    print "OK\n";
    return "";
}

sub TREE_BUILD {
    print "Building extract1 tree..";
    my $ret;
    $ret = `cd extract1 && make >../logs/TREE_BUILD.log 2>&1 || echo -n \$?`;
    return "Tree build failed ($ret)"
        unless $ret eq "";

    print "OK\n";
    print "Removing extract1 tree..";

    $ret = `rm -r extract1  || echo -n \$?`;
    return "Tree deletion failed ($ret)"
        unless $ret eq "";
    print "OK\n";

    return "";
}

sub SECOND_EXTRACT {
    print "Extracting files for distribution..";
    my $extractPath = "ULAM/ulam1/share/perl/extractDistro.pl";
    my $distroName = "ulam-$ulam_version_tag";
    my $ret = `$extractPath src ULAM $distroName >logs/SECOND_EXTRACT.log 2>&1 || echo \$?`;
    return "Second extract failed ($ret)"
        unless $ret eq "";

    print "Removing Makefile.local.mk..";
    my $del = "$distroName/ULAM/Makefile.local.mk" ;
    unlink $del or return "Couldn't unlink $del: $!";

    my $tarPath = "$distroName.tgz";
    print "Making $tarPath..";
    $ret = `tar cvfz $tarPath $distroName >logs/SECOND_EXTRACT-tar.log 2>&1 || echo \$?`;
    return "Building $tarPath failed ($ret)"
        unless $ret eq "";
    print "OK\n";
    return "";
}

sub DISTRO_BUILD {
    my $distroName = "ulam-$ulam_version_tag";
    my $tarPath = "$distroName.tgz";

    my $cmd = "bzr dh-make --bzr-only ulam $ulam_version_tag $tarPath >logs/DISTRO_BUILD-dhmake.log 2>&1 || echo \$?";
    print "Running [$cmd]..";
    my $ret = `$cmd`;
    return "[$cmd] failed ($ret)"
        unless $ret eq "";
    print "OK\n";

    my $newppaversion = makeLeximited(incrementFileNumber("$STATE_DIR/ulam-ppaversion"));
    for my $distro (@DISTROS) {
        print "\n\n------Packaging for $distro------\n";

        my $changelogdate = `date +"%a, %d %b %Y %T %z"`;
        chomp($changelogdate);

        my $newdistroversion = makeLeximited(incrementFileNumber("$STATE_DIR/ulam-$distro-version"));
        my $debianversion = "$ulam_version_tag-ppa$newppaversion~$distro$newdistroversion";
        print "Updating debian/changelog for $debianversion..";
        open(LOG,">ulam/debian/changelog") or die "LOG: $!";
        print LOG <<EOF;
ulam ($debianversion) $distro; urgency=low

  * Packaging ulam $ulam_version_tag for $distro

 -- Dave Ackley <ackley\@ackleyshack.com>  $changelogdate
EOF
        close LOG or die "Closing $!";
        print "done\n";

        print "Committing updated debian in bzr..";
        $cmd = "cd ulam && bzr add debian && bzr commit -m \"Packaging $ulam_version_tag for $distro on $changelogdate\"";
        $ret = `$cmd >../logs/DISTRO_BUILD-update-debian.log 2>&1 || echo \$?`;
        return "[$cmd] failed ($ret)"
            unless $ret eq "";
        print "OK\n";

        print "Building source package..";
        $cmd = "cd ulam;bzr builddeb -S";
        $ret = `$cmd >../logs/DISTRO_BUILD-source-package.log 2>&1 || echo \$?`;
        return "[$cmd] failed ($ret)"
            unless $ret eq "";
    }
    return "";
}

sub initFileNumber {
    my $path = shift;
    open(WFILE,">$path") or die "Can't write '$path': $!";
    print WFILE "0\n";
    close WFILE or die "Closing $!";
}

sub incrementFileNumber {
    my $path = shift;
    while (!open(NFILE,"<$path")) {
        initFileNumber($path);
    }
    my $num = <NFILE>;
    close(NFILE) or die "$!";
    chomp($num);
    $num =~ /^\s*([0-9]+)\s*$/
        or die "No match num '$num' in '$path'";
    my $rnum = $1;
    ++$rnum;
    open(NFILE,">$path") or die "Writing '$path': $!";
    print NFILE "$rnum\n";
    close(NFILE) or die "$!";
    return $rnum;
}

sub makeLeximited {
    my $num = shift;
    my $len = length($num);
    return "$len$num" if $len < 9;
    return "9".makeLeximited($len).$num;
}
