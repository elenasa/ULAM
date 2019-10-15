#!/usr/bin/perl -w

use Time::Local;
use File::Copy;
use File::Path;
use strict;
use warnings;
use Cwd;

use File::Spec;
use Cwd 'abs_path';

my $SCRIPT_PATH;
my $ULAM_ROOT;
my $CULAM_PATH;
my $ULAM_SHARE_DIR;

BEGIN {
    use File::Basename;

    $SCRIPT_PATH = __FILE__;
    $SCRIPT_PATH = File::Spec->rel2abs( readlink $SCRIPT_PATH, dirname($SCRIPT_PATH))
        if -l $SCRIPT_PATH;

    $ULAM_ROOT = abs_path(dirname($SCRIPT_PATH)."/../..");

    # Note we're not allowing spaces in the path.  F-U if that's a problem.
    if ($ULAM_ROOT =~ /^([^" \*\?\[\]\$\|<>&%()!;\\]+)$/) {
    	$ULAM_ROOT = $1;
    } else {
    	die "Bad characters in path '$ULAM_ROOT'";
    }

    $CULAM_PATH = "$ULAM_ROOT/bin/culam";
    $ULAM_SHARE_DIR = "$ULAM_ROOT/share";

    push @INC, "$ULAM_SHARE_DIR/perl";
}

my $TOPLEVEL = "$ULAM_ROOT";
my $TESTDIR =  "$ULAM_ROOT/src/test/generic";
my $TESTBIN =  "/tmp";
my $SRC_DIR = "safe";
my $DEBUG = 0;

sub usage_abort
{
    my $msg = shift;
    print STDERR "ERROR: $msg\n" if defined $msg;
    die "Usage: $0 -s [TESTFILE..], default is all safe *.test files.\n";
}

if (scalar(@ARGV) > 0 && $ARGV[0] eq "-s") {

    shift @ARGV; # next arg specifies test file (or pattern)
}

&main(@ARGV);

sub main
{
    my $arg = $_[0];
    my $query = "*.test";

#allow user to specify one test pattern: e.g. t3232, t412
    if(@_ > 0)
    {
	if($arg =~ /t[0-9]?/ )
	{
	    $query=$arg . "*.test";
	}
	else
	{
	    usage_abort();
	}
    }
    else { }

    my $dir = getcwd;
    usage_abort("Not at top-level.  Please run this script from your TOPLEVEL: <$TOPLEVEL>")
        unless $dir =~ $TOPLEVEL;

    my $start = time;
    my $N = 0; # number of tests run

    my @files = <$TESTDIR/$SRC_DIR/$query>;
    chomp @files;

    my $fileCount = scalar(@files);
    usage_abort("No tests matching '$TESTDIR/$SRC_DIR/$query'")
        unless $fileCount > 0;

    print "Found $fileCount test files\n";

    my $f;
    my $lasttestnum = 0;

    my $testsPassed = 0;
    my $testsFailed = 0;

    foreach $f (sort @files) {
	if ($f =~ /t(\d+)(.*)\.test$/ ) {
	    my $testnum = $1;
	    (int($testnum) == int($lasttestnum)) && next;
	    #print "$f, $lasttestnum, $testnum\n";
	    $lasttestnum = $testnum;

	    my $testf = "t" . $testnum . $2 . ".test";
	    my $src = $TESTDIR . "/$SRC_DIR/". $testf;
	    #print "src: <$src>\n";
	    #my $newsrc = $TESTBIN . "/" . $testf;
	    my $newsrc = $TESTDIR . "/$SRC_DIR". "7/" . $testf;

	    my $log = "/tmp/t" . $testnum . "-testlog.txt";
	    my $errlog = "/tmp/t" . $testnum . "-testerrlog.txt";

	    open OUTTEST, ">$newsrc" || die "failed open for write <$newsrc>";
	    open INTEST, "<$src" || die "failed open for reading <$src>";
	    open GOT,"<$errlog" || die "failed to open <$errlog>";

	    my $copyline = 0;
	    my $replace = 0;
	    my $bypassgot = ((-s $errlog) == 0);
	    $DEBUG && print "bypass is $bypassgot\n";

	    while(my $line = <INTEST>){
		if( $line =~ /^#\!/ ) {
		    $copyline = 0;
		    $replace = 1;
		    print OUTTEST $line;
		}elsif( $line =~ /^#/ ){
		    $copyline = 1;
		    $replace = 0;
		}

		if($copyline || $bypassgot){
		    $DEBUG && print "$testf copy: <$line>\n";
		    print OUTTEST $line;
		} elsif ($replace) {

		    while(my $ans = <GOT>){

			if( $ans =~ /INVALID/ ){
			    $DEBUG && print "t$testnum match starts..\n";
			    next;
			} elsif( $ans =~ /^]/ ){
			    $DEBUG && print "t$testnum match ends.\n";
			    $replace = 0; #done
			    last;
			} elsif( $ans =~ /^culamtest:/ )
			{
			    $DEBUG && print "t$testnum failed, copy: <$line>\n";
			    print OUTTEST $line;
			    $bypassgot = 1;
			    last;
			} else {
			    $DEBUG && print "t$testnum new ans: <$ans>\n";
			    print OUTTEST $ans;
			}
		    }
		} # end replace
	    }

	    close GOT;
	    close OUTTEST;
	    close INTEST;

	    $DEBUG && print "done with t$testnum\n";
	    $N++;
	} else {
	    print "no match for <$f>\n";
	}
    } # end for loop

    my $lapse = time - $start;
    my $pertest = int($lapse/$N * 1000) / 1000.;
    my $perfect = $testsPassed == $N;
    if ($perfect) {
        print "SUCCESS: Passed $testsPassed of $N in $lapse seconds ($pertest sec/test)\n";
    } else {
        print "FAILURE: Passed $testsPassed of $N in $lapse seconds ($pertest sec/test)\n";
    }
    exit($perfect ? 0 : 1);
} #end main
