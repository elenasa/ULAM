#!/usr/bin/perl -w

BEGIN {
    use FindBin;
    push @INC, $FindBin::Bin;
}

my $version = "1.0";

use ULAMScanner;

my $file = shift @ARGV;

if (defined($file) && $file eq "-V") {
    print "$FindBin::Script $version\n";
    exit(0);
}

my $className = shift @ARGV;
my $mangledName = shift @ARGV;

die "Usage: $0 Foo.ulam FooClassName Ue_FooMangledName\n"   unless defined $mangledName;
die "Too many arguments\n"                unless !scalar(@ARGV);

die "Not an ulam file '$file'\n" unless $file =~ /[.]ulam$/;
die "Can't read $file: $!\n"     unless open(HANDLE, "<", "$file");

my $us = ULAMScanner->new();
$us->load(*HANDLE);

die "Failed closing '$file': $!"unless close(HANDLE);

$us->scan();

die "Malformed filename '$file'\n"
    unless $file =~ m/^.*?([A-Z][A-Za-z0-9]*)[.]ulam$/;

die "Malformed class name '$className'\n"
    unless $className =~ m/^[A-Z][A-Za-z0-9]*$/;

my $doc = $us->getClassDoc($className);

my %keys = $us->analyzeDoc($className, $doc);
%keys = $us->normalizeKeys(%keys);

my $stamp = gmtime(time());
my $cname = $us->makeCString($className);

my $csym = $us->makeCString($keys{'symbol'});
my $csum = $us->makeCString($us->standardizeWhitespace($keys{'brief'}));
my $cdet = $us->makeCString($us->standardizeWhitespace($keys{'detail'}));

my $movfunc = "";
my $pct = $keys{'diffusability'};
if (defined $pct) {
    $movfunc = <<EOF;
    virtual const u32 GetPercentDiffusability() const
    {
      return $pct;
    }
EOF
}

my $author = $us->makeCString($keys{'author'});
my $license = $us->makeCString($keys{'license'});
my $cver = $keys{'version'};
my $cplaceable = "true";
$cplaceable = "false" if $keys{'placeable'} eq "no";

my @colors = split(/,/,$keys{'colors'});
my $cnum = scalar(@colors);
my $body = "";
## If any color slots say 'function', use 'function' for all missing slots as well
my $haveFunction = 0;
for (my $i = 0; $i < $cnum; ++$i) {
    my $c = $colors[$i];
    if ($c eq "function") {
        $haveFunction = 1;
    }
}
$cnum = 5 if ($haveFunction && $cnum < 5);
for (my $i = 0; $i < $cnum; ++$i) {
    my $c = $colors[$i];
    if (!defined($c) || $c eq "function") {
        $body .= "\n      case $i: {  // And woe unto you if you didn't define this!" .
                 "\n                 return m_ulamElement.Uf_8getColor(uc,atom,Ui_Ut_102328Unsigned(colnum)).read();".
                 "\n               }"
    } else {
        $body .= "\n      case $i: return $colors[$i];"
    }
}
my @syms = split(/,/,$keys{'symmetries'});
my $csyc = scalar(@syms);
my $symbody = "";
if ($csyc == 1) {
    $symbody .= "return $syms[0];";
} else {
    $symbody .= <<EOF;
      Random & r = uc.GetRandom();
EOF
    if ($csyc == 8) {
      $symbody .= "      return r.Create($csyc);"
    } else {
      $symbody .= <<EOF;
      switch (r.Create($csyc)) {
EOF
    $symbody .= "        default: /* not reached */";
    for (my $i = 0; $i < $csyc; ++$i) {
        $symbody .= "\n        case $i: return $syms[$i];"
    }
    $symbody .= "\n      }";
   }
}
    my $structName = "UlamElementInfoFor$mangledName";
print <<EOF;
$structName
/* This is generated code!  Avoid hand editing!
   The content of this file is based on the information
   found in '$file'.
   Make any desired changes there!

   Generated on $stamp UTC
   by the ULAM compilation system
*/

#include "UlamDefs.h"

namespace MFM {
  template <class EC>
  struct $structName : public UlamElementInfo<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    const UlamElement<EC> & m_ulamElement;
    $structName(const UlamElement<EC> & ue)
        : m_ulamElement(ue)
    { }

    $structName() { }
    const char * GetName() const { return $cname; }
    const char * GetSymbol() const { return $csym; }
    const char * GetSummary() const { return $csum; }
    const char * GetDetails() const { return $cdet; }
    const char * GetAuthor() const { return $author; }
    const char * GetLicense() const { return $license; }
    bool GetPlaceable() const { return $cplaceable; }
    const u32 GetVersion() const { return $cver; }
$movfunc
    const u32 GetNumColors() const { return $cnum; }
    const u32 GetColor(UlamContext<EC>& uc, T atom, u32 colnum) const {
      switch (colnum) {
      default: $body
      }
    }
    const u32 GetSymmetry(UlamContext<EC>& uc) const {
      $symbody
    }
  };
} // MFM

EOF
