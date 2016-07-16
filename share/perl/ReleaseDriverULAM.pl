#!/usr/bin/perl -w
#XXX FOR RELEASE USE THIS:    
# my $extractPath = "ULAM/share/perl/extractDistro.pl";
#XXX FOR HACKING ON extractDistro.pl USING THIS:
my $extractPath = "/home/ackley/papers/MF/asrepo/RT13/code/ULAM-fork/ULAM/share/perl/extractDistro.pl";

my ($ULAMTAG, $MFMTAG, $PACKAGENAME); processArgs(@ARGV);
my %steps; sub step { $steps{scalar(keys %steps).". ".$_[0]} = $_[1]; }
#
# This script runs as the main event of the ULAM_DISTRO_CREATION_TIME
# era.  It does the following steps:
#
step("REPO_CHECK_OUT",<<EOS);

  Checks out the MFM and ULAM repos using the given tags, arranging
  them as siblings in the work dir.

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
  builds it using bzr / dh-make

EOS

## PRELIMINARIES TO ALL STEPS

my $ULAM_LANGUAGE_VERSION="ulam2";

$| = 1;
my $GIT_URL = "https://github.com/DaveAckley/ULAM.git";
my $MFM_GIT_URL = "https://github.com/DaveAckley/MFM.git";
my @DISTROS = ("precise", "trusty", "xenial");

use Cwd 'abs_path';
use File::Basename;
use File::Temp qw/ tempfile tempdir /;

my $MY_PATH = abs_path($0);
my $MY_DIR = dirname($MY_PATH);

my $STATE_DIR = `readlink -f ~/pbuilder/localState`;
chomp($STATE_DIR);

my $mfm_version_tag = undef; # Set in step REPO_BUILD
my $ulam_version_tag = undef; # Set in step REPO_BUILD

my $workBaseDir = tempdir( "$PACKAGENAME-XXXXXX", TMPDIR => 1, CLEANUP => 0 );
mkdir("$workBaseDir/work") || die "$!";
my $workDir = "$workBaseDir/work/$PACKAGENAME";
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
    print "Checking out $ULAMTAG..";
    my $git_clone_output2 = `cd ULAM;git checkout $ULAMTAG 2>&1`;
    print "done\n";

    print "Cloning repo $MFM_GIT_URL..";
    my $git_mfm_clone_output = `git clone $MFM_GIT_URL 2>&1`;
    print "done\n";
    print "Checking out $MFMTAG..";
    my $git_mfm_clone_output2 = `cd MFM;git checkout $MFMTAG 2>&1`;
    print "done\n";

    print "Making Makefile.local.mk..";
    `echo 'MFM_ROOT_DIR := \$(ULAM_ROOT_DIR)/../MFM' > ULAM/Makefile.local.mk`;
    print "done\n";
    return "";
}

sub REPO_BUILD {
    print "Building MFM repo..";
    mkdir "logs" or die "mkdir: $!";
    my $ret;
    $ret = `cd MFM && COMMANDS=1 make >../logs/MFM_REPO_BUILD.log 2>&1 || echo -n \$?`;
    return "MFM repo build failed ($ret)"
        unless $ret eq "";
    print "OK\n";


    print "Building ULAM repo..";
    $ret = `cd ULAM && COMMANDS=1 make >../logs/ULAM_REPO_BUILD.log 2>&1 || echo -n \$?`;
    return "ULAM repo build failed ($ret)"
        unless $ret eq "";
    print "OK\n";

    print "Compiling standard elements..";
    $ret = `cd ULAM && COMMANDS=1 make ulamexports >../logs/ULAMEXPORTS_REPO_BUILD_EXPORTS.log 2>&1 || echo -n \$?`;
    return "Repo .so build failed ($ret)"
        unless $ret eq "";
    print "OK\n";

    print "Getting version number from MFM..";
    $mfm_version_tag = `cd MFM;make version`;
    chomp($mfm_version_tag);
    $mfm_version_tag =~ /^\d+[.]\d+[.]\d+$/ or return "MFM version not found, got '$mfm_version_tag'";
    print "$mfm_version_tag\n";

    print "Getting version number from ULAM..";
    $ulam_version_tag = `cd ULAM;make -f Makedebian.mk --quiet version`;
    chomp($ulam_version_tag);
    $ulam_version_tag =~ /^\d+[.]\d+[.]\d+$/ or return "Ulam version not found, got '$ulam_version_tag'";
    print "$ulam_version_tag\n";

    print "Getting git version tags\n";
    my $f;
    $f = "ULAM/ULAM_TREEVERSION.mk";
    if (-r $f) { print " ".`cat $f`; } else { return "Not found '$f'"; }
    $f = "MFM/MFM_TREEVERSION.mk";
    if (-r $f) { print " ".`cat $f`; } else { return "Not found '$f'"; }

    return "";
}

sub FIRST_EXTRACT {
    print "Extracting files for test build..";

    my $ret = `$extractPath src ../$PACKAGENAME extract1 >logs/FIRST_EXTRACT.log 2>&1 || echo \$?`;
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
    my $distroName = "ulam-$ulam_version_tag";
    my $ret = `$extractPath src ../$PACKAGENAME $distroName >logs/SECOND_EXTRACT.log 2>&1 || echo \$?`;
    return "Second extract failed ($ret)"
        unless $ret eq "";
    print "OK\n";

    print "Removing Makefile.local.mk..";
    my $del = "$distroName/ULAM/Makefile.local.mk" ;
    unlink $del or return "Couldn't unlink $del: $!";
    print "OK\n";

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

    my $cmd = "bzr dh-make --bzr-only $PACKAGENAME $ulam_version_tag $tarPath >logs/DISTRO_BUILD-dhmake.log 2>&1 || echo -n \$?";
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
        print "Updating $PACKAGENAME/ULAM/debian/changelog for $debianversion..";
        open(LOG,">$PACKAGENAME/debian/changelog") or die "LOG: $!";
        print LOG <<EOF;
$PACKAGENAME ($debianversion) $distro; urgency=low

  * Packaging $PACKAGENAME $ulam_version_tag for $distro

 -- Dave Ackley <ackley\@ackleyshack.com>  $changelogdate
EOF
        close LOG or die "Closing $!";
        print "done\n";

        print "Committing updated debian in bzr..";
        $cmd = "cd $PACKAGENAME && bzr add debian && bzr commit -m \"Packaging $ulam_version_tag for $distro on $changelogdate\"";
        $ret = `$cmd >../logs/DISTRO_BUILD-update-debian.log 2>&1 || echo -n \$?`;
        return "[$cmd] failed ($ret)"
            unless $ret eq "";
        print "OK\n";

        print "Building source package..";
        $cmd = "cd $PACKAGENAME && bzr builddeb -S";
        $ret = `$cmd >../logs/DISTRO_BUILD-source-package.log 2>&1 || echo -n \$?`;
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

sub checkVersionSyntax {
    my $v = shift;
    $v =~ /^v\d+([.]\d+)*$/
        or die "Illegal version '$v', not 'v1', 'v2.3', 'v2.4.1' etc\n";
}
sub checkPackageSyntax {
    my $p = shift;
    $p =~ /^[a-zA-Z][a-zA-Z0-9]{0,15}/
        or die "Unexpected package name '$p', want alpha + alphanum{0,15}\n";
}

sub processArgs {
    die "USAGE: $0 ULAMTAG MFMTAG PACKAGENAME\n"
        unless scalar(@_) == 3;
    $ULAMTAG = $_[0];
    $MFMTAG = $_[1];
    $PACKAGENAME = $_[2];

    checkVersionSyntax($ULAMTAG);
    checkVersionSyntax($MFMTAG);
    checkPackageSyntax($PACKAGENAME);
}
