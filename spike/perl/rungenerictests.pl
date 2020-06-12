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
my $EXEC_TEST_PERFORMANCE = 0;
my $EXEC_PTEST_VALGRIND = 0;  #=1 produces uncomparable log files (grep for "leaked" in them).
my $PERF_LOOP = 1;
my $SRC_DIR = "safe";
#my $SRC_DIR = "error"; # comment out Node::countNavNodes second line of message for comparison.
#my $SRC_DIR = "fail"; # runtime failures w testgencode
my $TESTGENCODE = 0; # 0 is faster; 1 is thorough

sub usage_abort
{
    my $msg = shift;
    print STDERR "ERROR: $msg\n" if defined $msg;
    die "Usage: $0 [-s[S|E|F|Pn][G][V]] TESTFILE.., default is all safe *.test files, S safe test (default), E error test, F fail test, G with GenCode testing, V with Valgrind testing, P with Performance GenCode-only testing n times per main loop.\n";
}

my $needRebuild = 1;
#if (scalar(@ARGV) > 0 && $ARGV[0] eq "-s") {
if (scalar(@ARGV) > 0) {
    if( $ARGV[0] =~ /([SEFP])/ ){
	$SRC_DIR = "safe" if ($1 eq 'S'); # default
	$SRC_DIR = "error" if ($1 eq 'E');
	$SRC_DIR = "fail" if ($1 eq 'F');
#	$SRC_DIR = "safe" if ($1 eq 'P'); #maybe 'perf' in future?
	$SRC_DIR = "perf" if ($1 eq 'P'); #'perf' uses share, not spike natives
    }

    if( $ARGV[0] =~ /P/ ){
	$EXEC_TEST_PERFORMANCE = 1;
	if( $ARGV[0] =~ /P([0-9])/ ){
	    $PERF_LOOP = $1;
	} # else default is 1 time thru loop
	elsif( $ARGV[0] =~ /V/ ){
	    $EXEC_PTEST_VALGRIND = 1;
	    $PERF_LOOP = 1;
	    print "Valgrind Performance main\n";
	}
	print "$PERF_LOOP times around main loop\n";
    } else {
	# performance test distinct from other tests
	if( $ARGV[0] =~ /G/ ){
	    $TESTGENCODE = 1; #default faster mode
	}
	if( $ARGV[0] =~ /V/ ){
	    $EXEC_TEST_VALGRIND = 1; #default faster mode
	}
    }


    if( $ARGV[0] =~ /^-s/ ) {
	$needRebuild = 0;
	shift @ARGV; # next arg specifies test file (or pattern)
    }
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

    `make -C $TESTDIR initdirs`; #before test

    foreach $f (sort @files) {
	if ($f =~ /t(\d+).*\.test$/ ) {
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
	    my $perflog = "/tmp/t" . $testnum . "-testnewperflog.txt";

	    # useful System Quark:
	    #`cp $TESTDIR/$SRC_DIR/t3207_test_compiler_quarksystem_inside_a_quark.cpp $TESTDIR/.`;

	    `make -C $TESTDIR clean`; #before test

	    if($EXEC_TEST_VALGRIND) {
		# one at a time, results in testerrlog
		# grep for:
                # All heap blocks were freed -- no leaks are possible

                ## useful to find source of error
                ## --track-origins=yes
                ## --leak-check=full
		`valgrind ./bin/culamtest $f 1> $log 2> $errlog`;
                my $status = $?;
                if ($status == 0) {
                    ++$testsPassed;
                } else {
                    ++$testsFailed;
                    print "FAILED: $testnum\n";
                }
                ## compile and run generated code
		if($TESTGENCODE){
		    `make -C $TESTDIR gen`;
		    if($EXEC_TEST_VALGRIND) {
			`valgrind $TESTBIN/main 1>> $log 2>> $errlog`;
		    }
		    else {
			`$TESTBIN/main 1>> $log 2>> $errlog`;
		    }
		}
	    } elsif($EXEC_TEST_PERFORMANCE) {
		# clears output/err log files; drops any output
		`./bin/culamtestperf $f 1>$log 2>$errlog`;

                my $status = $?;
                if ($status != 0) {
                    ++$testsFailed;
                    print "FAILED culamtestperf: $testnum\n";
                } else {
		    ## compile and run generated code without eval;
		    ## timing into perf log
		    `make -C $TESTDIR perf`;

		    $status = $?;
		    if ($status != 0) {
			++$testsFailed;
			print "FAILED: $testnum\n";
		    } else {
			if($EXEC_PTEST_VALGRIND) {
			    `valgrind $TESTBIN/main $PERF_LOOP 1>/dev/null 2>>$perflog`;
			    ++$testsPassed;
			} else {
			    ## append or not to append??
			    `$TESTBIN/main $PERF_LOOP 1>/dev/null 2>>$perflog`;
			    ++$testsPassed;
			    print "got timing for t$testnum\n"; #progress..
			}
		    }
		}
	    } else {
		if($SRC_DIR =~ /error/) {
		    `./bin/culamtest $f 1>/dev/null`;  ##outputs errlog to stderr
		} else {
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
		if($TESTGENCODE) {
		    `make -C $TESTDIR gen`;
		    if($EXEC_TEST_VALGRIND) {
			`valgrind $TESTBIN/main 1>> $log 2>> $errlog`;
		    } else {
			`$TESTBIN/main 1>> $log 2>> $errlog`;
		    }
		}
	    }

	    $TESTGENCODE && print "done with t$testnum\n";
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
