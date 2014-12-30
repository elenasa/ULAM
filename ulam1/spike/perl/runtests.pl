#!/usr/bin/perl -w

use File::Copy;
use File::Path;
use strict;
use warnings;
use Cwd;

# Figure out where we were executed from and add that directory to the
# include path.  This is unwise in that it opens the possibility of
# sucking in anything included in the directory we're in, not just the
# stuff we want to allow sucking in..
sub BEGIN {
    # Seal us up a bit for living la vida tainted
    $ENV{'PATH'} = "/bin:/usr/bin";
    delete @ENV{'IFS', 'CDPATH', 'ENV', 'BASH_ENV'};

    my $dir = ($0 =~ m!^(.*)/!);
    if ($dir !~ m!^/!) {my $tmp = `pwd`; chop($tmp);}
    @INC=($dir, @INC) unless $INC[0] eq $dir;
}

my $TOPLEVEL = "/home/elenas/WORK/ulam/repo/ULAM/ulam1";
my $TESTDIR =  "/home/elenas/WORK/ulam/repo/ULAM/ulam1/src/test";
my $EXEC_TEST = 1;

sub usage_abort
{
    die "Usage: perl runtests.pl <t3nnn>, default is all t3* tests.\n";
}

&main(@ARGV);

sub main {
    my $arg = $_[0];
    my $query = "t3*.cpp";

#allow user to specify one test: e.g. t3232
    if(@_ > 0)
    {
	if($arg =~ /t[0-9]?/ )
	{
	    $query=$arg . "*.cpp";
	}
	else
	{
	    &usage_abort();
	}
    }
    else
    {
	print "ALL t3* tests will be run..logs will be available in /tmp as they complete..your patience is appreciated.\n";
    }

    my $dir = getcwd;
    print "<$dir>\n";
    if( $dir !~ $TOPLEVEL)
    {
	print "Please run this script from your TOPLEVEL: <$TOPLEVEL>\n";
	die;
    }

    my @files = <$TESTDIR/safe/$query>;
    chomp @files;
    my $f;
    my $lasttestnum = 0;

    foreach $f (sort @files) {
    if ($f =~ /t(\d{4}).*\.cpp$/ )
    {
	my $testnum = $1;
	(int($testnum) == int($lasttestnum)) && next;
	print "$f, $lasttestnum, $testnum\n";
	$lasttestnum = $testnum;

	my $dest = $TESTDIR;
	my $testf = "t" . $testnum . "*.cpp";
	my $src = $TESTDIR . "/safe/". $testf;
	print "src: <$src> , dest: <$dest> \n";

	my $log = "/tmp/t" . $testnum . "-testlog.txt";
	my $errlog = "/tmp/t" . $testnum . "-testerrlog.txt";

	# useful System Quark:
	`cp $TESTDIR/safe/t3207_test_compiler_quarksystem_inside_a_quark.cpp $TESTDIR/.`;

	if($EXEC_TEST)
	{
	    `cp $src $dest 1> $log 2> $errlog`;
	    `COMMANDS=1 make -C $TOPLEVEL realclean 1>> $log 2>> $errlog`;
	    `COMMANDS=1 make -C $TOPLEVEL 1>> $log 2>> $errlog`;
	    `valgrind ./bin/test 1>> $log 2>> $errlog`;
	    `COMMANDS=1 make -C $TESTDIR gen 1>> $log 2>> $errlog`;
	    `tail -n 10 $errlog`;
	    `tail -n 10 $log`;
            #clean up
	    $dest = $dest . "/" . $testf;
	    `rm -f $dest`;
	}

	print "done with $testnum\n";

    } else {
	print "no match for <$f>\n";
    }

    }
    return;
}

