#!/usr/bin/perl -w

BEGIN {
    use FindBin;
    push @INC, $FindBin::Bin;
}

my $version = "1.0";

use ULAMScanner;

my $file = shift @ARGV;

die "Usage: $0 Foo.ulam\n"       unless defined $file;
die "Too many arguments\n"       unless !scalar(@ARGV);

if ($file eq "-V") {
    print "$FindBin::Script $version\n";
    exit(0);
}

die "Not an ulam file '$file'\n" unless $file =~ /[.]ulam$/;
die "Can't read $file: $!\n"     unless open(HANDLE, "<", "$file");

my $us = ULAMScanner->new();
$us->load(*HANDLE);

die "Failed closing '$file': $!"unless close(HANDLE);

$us->scan();

die "Malformed filename '$file'\n"
    unless $file =~ m/^.*?([A-Z][A-Za-z0-9]*)[.]ulam$/;

my $className = $1;
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

my @colors = split(/,/,$keys{'colors'});
my $cnum = scalar(@colors);
my $body = "";
for (my $i = 0; $i < $cnum; ++$i) {
    my $c = $colors[$i];
    if ($c eq "function") {
        $body .= "\n      case $i: {  // And woe unto you if you didn't define this!" .
                 "\n                 UlamContext<CC> uc;".
                 "\n                 return m_ulamElement.Uf_8getColor(uc,atom,Ui_Ut_102328Unsigned($i)).read();".
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
UlamContext<CC> &uc = UlamContext<CC>::Get();
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
    my $structName = "UlamElementInfoFor$className";
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
  template <class CC, template <class> class UE>
  struct $structName : public UlamElementInfo<CC>
  {
    const UE<CC> & m_ulamElement;
    typedef typename CC::ATOM_TYPE T;
    $structName(const UE<CC> & ue)
        : UlamElementInfo<CC>(ue)
        , m_ulamElement(ue)
    { }
    const char * GetName() const { return $cname; }
    const char * GetSymbol() const { return $csym; }
    const char * GetSummary() const { return $csum; }
    const char * GetDetails() const { return $cdet; }
    const char * GetAuthor() const { return $author; }
    const char * GetLicense() const { return $license; }
    const u32 GetVersion() const { return $cver; }
$movfunc
    const u32 GetNumColors() const { return $cnum; }
    const u32 GetColor(T atom, u32 colnum) const {
      switch (colnum) {
      default: $body
      }
    }
    const u32 GetSymmetry() const {
      $symbody
    }
  };
} // MFM

EOF
