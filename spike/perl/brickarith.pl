#!/usr/bin/perl -w

use Cwd;
use Time::Local;
use File::Temp;
use File::Copy;
use strict;
use warnings;

######################################################################
#
# brickarith.pl
#
# Author: E.S. Ackley
# Created: 09/19/2017
# Updated: 10/10/2017
#
# The purpose of this script is test brick arithmetic for MFM
#
# Given a SiteCoord s.
#   0. validate s is legal place to have an event (i.e. not in cache-space)
#   1. find its neighbors who share the event window (0-2 of Directions {NW,NE,SW,SE,W,E} )
#   2. map s foreach neighbor who shares the event.
#
# Output:
# Failure message if invalid event location;
# No neighbors share this event;
# or foreach neighbor
#    Direction : S(mappedX, mappedY)
#
# SLOGAN: our share is neighbors cache; our cache is neighbors share;
#
#  hidden, visible, share, cache (last 3 are each EventWindowRadius wide)
#
#############################################################################

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

my $version = "0.03";
my $debug = 0;

##################
##
## Constants
##
##################
my $WIDTH = 64;
my $HEIGHT = 32;
my $EW_RADIUS = 4;
my $CACHE = $EW_RADIUS;
my $SHARED = $EW_RADIUS * 2;
my $VISIBLE = $EW_RADIUS * 3;

my $cN = 0; #unused in bricks
my $cNE = 1;
my $cE = 2;
my $cSE = 3;
my $cS = 4; #unused in bricks
my $cSW = 5;
my $cW = 6;
my $cNW = 7;
my @DIRSTR = ( "N", "NE", "E", "SE", "S", "SW", "W", "NW" );

###########################################################
##
## Lookup Tables for mapping x and y, indexed by direction
##
###########################################################
#my @DirMapX = ( 0, -$WIDTH/2, -$WIDTH, -$WIDTH/2, 0, $WIDTH/2, $WIDTH, $WIDTH/2 );
#my @DirMapY = ( 0, $HEIGHT, 0, -$HEIGHT, 0, -$HEIGHT, 0, $HEIGHT );
my @DirMapX = ( 0, -1, -2, -1, 0, 1, 2, 1 ); # = * Width / 2
my @DirMapY = ( 0, 2, 0, -2, 0, -2, 0, 2 );  # = * Height / 2

&main(@ARGV);

sub isLegalEventCoord
{
# cache is not legal event center coord.
    my $siteX = shift;
    my $siteY = shift;
    if($siteX < $CACHED || $siteX >= $WIDTH - $CACHED)
    {
	return 0; # illegal in cache
    }
    if($siteY < $CACHED || $siteY >= $HEIGHT - $CACHED)
    {
	return 0; #illegal in cache
    }
    return 1; #ok!
} #isLegalEventCoord


sub findNeighbors
{
    my $siteX = shift; #event center
    my $siteY = shift; #event center
    my $rtnarr = shift;

    #print "findNeighbors: $siteX, $siteY\n";
    &isLegalEventCoord($siteX, $siteY) || die;
    if( ($siteX < $VISIBLE) && ($siteY < $VISIBLE))
    {
	#top left corner - shared
	push(@$rtnarr, $cNW);
	push(@$rtnarr, $cW);
    }
    elsif( ($siteX < $VISIBLE) && ($siteY >= $HEIGHT - $VISIBLE ))
    {
	#bottom left corner - shared
	push(@$rtnarr, $cSW);
	push(@$rtnarr, $cW);
    }
    elsif(($siteX < $VISIBLE) && ($siteY >= $VISIBLE) && ($siteY < $HEIGHT - $VISIBLE))
    {
	# left horizontal - shared
	push(@$rtnarr, $cW);
    }

    if( ($siteX >= $WIDTH - $VISIBLE) && ($siteY < $VISIBLE))
    {
	#top right corner - shared
	push(@$rtnarr, $cNE);
	push(@$rtnarr, $cE);
    }
    elsif( ($siteX >= $WIDTH - $VISIBLE) && ($siteY >= $HEIGHT - $VISIBLE))
    {
	#bottom right corner - shared
	push(@$rtnarr, $cSE);
	push(@$rtnarr, $cE);
    }
    elsif(($siteX >= $WIDTH - $VISIBLE) && ($siteY >= $VISIBLE) && ($siteY < $HEIGHT - $VISIBLE))
    {
	# right horizontal - shared
	push(@$rtnarr, $cE);
    }

    elsif( ($siteX >= $WIDTH/2 - $VISIBLE) && ($siteX < $WIDTH/2 + $VISIBLE) && ($siteY < $VISIBLE) )
    {
	# top "T" middle
	push(@$rtnarr, $cNW);
	push(@$rtnarr, $cNE);
    }
    elsif( ($siteX >= $WIDTH/2 - $VISIBLE) && ($siteX < $WIDTH/2 + $VISIBLE) && ($siteY >= $HEIGHT - $VISIBLE) )
    {
	# bottom "T" middle
	push(@$rtnarr, $cSW);
	push(@$rtnarr, $cSE);
    }

    elsif(($siteX >= $VISIBLE) && ($siteX < $WIDTH/2 - $VISIBLE) && ($siteY < $VISIBLE))
    {
	#top left vertical
	push(@$rtnarr, $cNW);
    }
    elsif(($siteX >= $EW_RADIUS) && ($siteX < $WIDTH/2 - $EW_RADIUS) && ($siteY >= $HEIGHT - $EW_RADIUS))
    {
	#bottom left vertical
	push(@$rtnarr, $cSW);
    }

    elsif(($siteX < $WIDTH - $VISIBLE) && ($siteX >= $WIDTH/2 + $VISIBLE) && ($siteY < $VISIBLE))
    {
	#top right vertical
	push(@$rtnarr, $cNE);
    }
    elsif(($siteX < $WIDTH - $VISIBLE) && ($siteX >= $WIDTH/2 + $VISIBLE) && ($siteY >= $HEIGHT - $VISIBLE))
    {
	#bottom right vertical
	push(@$rtnarr, $cSE);
    }
    #else hidden 0 neighbors share the event
    return 1;
} #findNeighbors

sub findNeighbors2
{
    my $siteX = shift;
    my $siteY = shift;
    my $rtnarr = shift;

    #print "findNeighbors2: $siteX, $siteY\n";

    &isLegalEventCoord($siteX, $siteY) || return 0;

    if( ($siteX < $VISIBLE))
    {
	if($siteY < $VISIBLE)
	{
	    #top left corner
	    push(@$rtnarr, $cNW);
	    push(@$rtnarr, $cW);
	}
	elsif($siteY >= $HEIGHT - $VISIBLE)
	{
	    #bottom left corner
	    push(@$rtnarr, $cSW);
	    push(@$rtnarr, $cW);
	}
	elsif(($siteY >= $VISIBLE) && ($siteY < $HEIGHT - $VISIBLE))
	{
	    # left horizontal
	    push(@$rtnarr, $cW);
	}
	else
	{
	    die;
	}
    }

    elsif( ($siteX >= $WIDTH - $VISIBLE))
    {
	if($siteY < $VISIBLE)
	{
	    #top right corner
	    push(@$rtnarr, $cNE);
	    push(@$rtnarr, $cE);
	}
	elsif(($siteY >= $HEIGHT - $VISIBLE))
	{
	    #bottom right corner
	    push(@$rtnarr, $cSE);
	    push(@$rtnarr, $cE);
	}
	elsif(($siteY >= $VISIBLE) && ($siteY < $HEIGHT - $VISIBLE))
	{
	    # right horizontal
	    push(@$rtnarr, $cE);
	}
	else
	{
	    die;
	}
    }

    elsif( ($siteX >= $WIDTH/2 - $VISIBLE) && ($siteX < $WIDTH/2 + $VISIBLE))
    {
	if($siteY < $VISIBLE)
	{
	    # top "T" middle
	    push(@$rtnarr, $cNW);
	    push(@$rtnarr, $cNE);
	}
	elsif($siteY >= $HEIGHT - $VISIBLE)
	{
	    # bottom "T" middle
	    push(@$rtnarr, $cSW);
	    push(@$rtnarr, $cSE);
	}
	#else hidden 0 neighbors share the event
    }

    elsif(($siteX >= $VISIBLE) && ($siteX < $WIDTH/2 - $VISIBLE))
    {
	if($siteY < $VISIBLE)
	{
	    #top left vertical
	    push(@$rtnarr, $cNW);
	}
	elsif($siteY >= $HEIGHT - $VISIBLE)
	{
	    #bottom left vertical
	    push(@$rtnarr, $cSW);
	}
	#else hidden 0 neighbors share the event
    }

    elsif(($siteX < $WIDTH - $VISIBLE) && ($siteX >= $WIDTH/2 + $VISIBLE))
    {
	if ($siteY < $VISIBLE)
	{
	    #top right vertical
	    push(@$rtnarr, $cNE);
	}
	elsif($siteY >= $HEIGHT - $VISIBLE)
	{
	    #bottom right vertical
	    push(@$rtnarr, $cSE);
	}
	#else hidden 0 neighbors share the event
    }
    #else hidden 0 neighbors share the event
    return 1;
} #findNeighbors2


sub mapSiteCoord
{
    my $dir = shift;
    my $siteX = shift;
    my $siteY = shift;

    my $mappedX = 0;
    my $mappedY = 0;

    #Table lookup instead (see mapSiteCoord2)
    if($dir == $cNE)
    {
	$mappedX = $siteX - $WIDTH/2;
	$mappedY = $siteY + $HEIGHT;
    }
    elsif($dir == $cNW)
    {
	$mappedX = $siteX + $WIDTH/2;
	$mappedY = $siteY + $HEIGHT;
    }
    elsif($dir == $cSE)
    {
	$mappedX = $siteX - $WIDTH/2;
	$mappedY = $siteY - $HEIGHT;
    }
    elsif($dir == $cSW)
    {
	$mappedX = $siteX + $WIDTH/2;
	$mappedY = $siteY - $HEIGHT;
    }
    elsif($dir == $cE)
    {
	$mappedX = $siteX - $WIDTH;
	$mappedY = $siteY;
    }
    elsif($dir == $cW)
    {
	$mappedX = $siteX + $WIDTH;
	$mappedY = $siteY;
    }
    else
    {
	die;
    }

    my @mappedSite = ();

    push(@mappedSite, $mappedX);
    push(@mappedSite, $mappedY);
    return @mappedSite;
} #mapSiteCoord

sub mapSiteCoord2
{
    my $dir = shift;
    my $siteX = shift;
    my $siteY = shift;

    if($dir == $cS || $dir == $cN || $dir > 7 || $dir < 0)
    {
	die;
    }

    #Table lookup for offset based on direction
    my $mappedX = $siteX + $DirMapX[$dir] * $WIDTH / 2;
    my $mappedY = $siteY + $DirMapY[$dir] * $HEIGHT / 2;

    my @mappedSite = ();

    push(@mappedSite, $mappedX);
    push(@mappedSite, $mappedY);
    return @mappedSite;
} #mapSiteCoord2

sub testxy
{
    my $xtest = shift;
    my $x = shift;
    my $y = shift;
    my @neighbors = ();
    if(&findNeighbors2($x,$y,\@neighbors))
    {
	print "X$xtest S($x,$y) maps:\n";
	my $n;
	foreach $n (sort @neighbors)
	{
	    my ($mappedX, $mappedY) = &mapSiteCoord2($n, $x, $y);
	    print "\t$DIRSTR[$n] : S($mappedX, $mappedY)\n";
	}

	if(scalar @neighbors == 0)
	{
	    print "\tNo neighbors share this event\n";
	}
    }
    else
    {
	print "Test X$xtest: Invalid Event location at S($x,$y)\n";
    }
    print "\n";
} #testxy

sub main
{
    #let's test brick_arithmetic

    #X1 Vertical-T-Right (32,0) NE:S(0,32), NW:S(64, 32)
    &testxy(1,32,0);

    #X2 Vertical-T-Left (28,3) NE:S(-4,35), NW:S(60, 35)
    &testxy(2,28,3);

    #X3 Vertical-Right-Top (36,3) NE:S(4,35)
    &testxy(3,36,3);

    #X4 Vertical-Left-Top (4,3) NW:S(36,35)
    &testxy(4,4,3);

    #X5 Corner-Right-Top (63,3) NE:S(31, 35), E:S(-1,3)
    &testxy(5,63,3);

    #X6 Corner-Left-Top (3,3) W:S(67,3) NW:S(35,35)
    &testxy(6,3,3);

    #X7 Corner-Left-Top (0,0) W:S(64,0) NW:S(32,32)
    &testxy(7,0,0);

    #X8 Hidden zone (none)
    &testxy(8,16,8);

    #X9 Cache zone - fails
    &testxy(9,-3, 0);

    #X10
    &testxy(10,63, 0);

    #X11
    &testxy(11,31, 0);
}#end main
