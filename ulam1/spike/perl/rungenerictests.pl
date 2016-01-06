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
my $TESTBIN =  "$ULAM_ROOT/src/test/bin";
my $EXEC_TEST_VALGRIND = 0;  #=1 produces uncomparable log files (grep for "leaked" in them).
my $SRC_DIR = "safe";
#my $SRC_DIR = "error"; # comment out Node::countNavNodes second line of message for comparison.
my $TESTGENCODE = 1; # 0 is faster; 1 is thorough

sub usage_abort
{
    my $msg = shift;
    print STDERR "ERROR: $msg\n" if defined $msg;
    die "Usage: $0 [-s] TESTFILE.., default is all *.test files.\n";
}

my $needRebuild = 1;
if (scalar(@ARGV) > 0 && $ARGV[0] eq "-s") {
    $needRebuild = 0;
    shift @ARGV;
}

&main(@ARGV);

sub main
{
    my $arg = $_[0];
    my $query = "*.test";

#allow user to specify one test: e.g. t3232
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
    else
    {
    }

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

    if ($needRebuild) {
	print "\n";
	print "Cleaning tree..";
	`COMMANDS=1 make -C $TOPLEVEL realclean 1>/tmp/toplevel-log.txt 2>/tmp/toplevel-errlog.txt`;

	print "done\nRebuilding tree (use '$0 -s' to skip clean/rebuild)..";
	`COMMANDS=1 make -C $TOPLEVEL 1>/tmp/toplevel-log.txt 2>/tmp/toplevel-errlog.txt`;
	print "done\n";
    }

    my $f;
    my $lasttestnum = 0;

    my $testsPassed = 0;
    my $testsFailed = 0;
    foreach $f (sort @files)
    {
	if ($f =~ /t(\d+).*\.test$/ )
	{
	    my $testnum = $1;
	    (int($testnum) == int($lasttestnum)) && next;
	    #print "$f, $lasttestnum, $testnum\n";
	    $lasttestnum = $testnum;

	    my $dest = $TESTDIR;
	    my $testf = "t" . $testnum . "*.test";
	    my $src = $TESTDIR . "/$SRC_DIR/". $testf;
	    #print "src: <$src> , dest: <$dest> \n";

	    my $log = "/tmp/t" . $testnum . "-testlog.txt";
	    my $errlog = "/tmp/t" . $testnum . "-testerrlog.txt";

	    # useful System Quark:
	    #`cp $TESTDIR/$SRC_DIR/t3207_test_compiler_quarksystem_inside_a_quark.cpp $TESTDIR/.`;

	    if($EXEC_TEST_VALGRIND)
	    {
		# one at a time, results in testerrlog
		# grep for:
                # All heap blocks were freed -- no leaks are possible

		$TESTGENCODE && `make -C $TESTDIR clean`; #before test

		`valgrind ./bin/culamtest $f 1> $log 2> $errlog`;
                my $status = $?;
                if ($status == 0) {
                    ++$testsPassed;
                } else {
                    ++$testsFailed;
                    print "FAILED: $testnum\n";
                }

                ## compile and run generated code
		if($TESTGENCODE)
		{
		    `make -C $TESTDIR gen`;
		    `$TESTBIN/main 1>> $log 2>> $errlog`;
		}

	    }
	    else
	    {
		$TESTGENCODE && `make -C $TESTDIR clean`; #before test

		if($SRC_DIR =~ /error/)
		{
		    `./bin/culamtest $f 1> $log`;  ##outputs errlog to stderr
		}
		else
		{
		    `./bin/culamtest $f 1> $log 2> $errlog`;
		}

                my $status = $?;
                if ($status == 0) {
                    ++$testsPassed;
                } else {
                    ++$testsFailed;
                    print "FAILED: $testnum\n";
                }

                ## compile and run generated code
		if($TESTGENCODE)
		{
		    `make -C $TESTDIR gen`;
		    `$TESTBIN/main 1>> $log 2>> $errlog`;
		}
	    }
	    $TESTGENCODE && print "done with t$testnum\n";
	    $N++;
	}
	else
	{
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
