package UlamGen;
use strict;
use warnings;
#202110101643 diagnostics is messing up with appimage-builder due to something causing
# 'couldn't find diagnostic data' so screw it
use diagnostics;
use Carp;

our $VERSION = 1.002;

sub new {
    my ($class, $name, $mangledName, $file) = @_;
    die "Missing arg(s)" unless defined $file and defined $mangledName;
    my $self = { };
    bless $self, $class;
    $self->{'classFile'} = $file;
    $self->{'className'} = $name;
    $self->{'classMangledName'} = $mangledName;
    $self->{'modelParameterList'} = [];
    return $self;
}

sub dump {
    my ($self, $handle) = @_;
    $handle = *STDERR unless defined $handle;
    print $handle "Dump($self)";
    $self->dumpHash($self, 1, $handle);
}

sub dumpHash {
    my ($self, $href, $depth, $handle) = @_;
    my $indent = "  "x$depth;
    for my $key (sort keys %{$href}) {
        my $val = $href->{$key};
        my $type = ref $val;
        if ($type eq "HASH") {
            print $handle "\n$indent$key=>";
            $self->dumpHash($val, $depth+1, $handle);
        } elsif ($type eq "ARRAY") {
            print $handle "\n$indent".join(", ", @{$val});
        } else {
            if (defined $val) {
                print $handle "\n$indent$key=>$val" ;
            } else {
                print $handle "\n$indent$key=>(undef)" ;
            }
        }
    }
    print $handle "\n";
}

sub addClassDoc {
    my ($self, $doc) = @_;
    $self->{'classScmt'} = $doc;
    my %keys = $self->analyzeDoc($doc);
    %keys = $self->normalizeCommonKeys(%keys);
    %keys = $self->normalizeClassKeys(%keys);
    $self->{'classKeys'} = \%keys;
}

sub addModelParameter {
    my ($self, $uname, $mangledType, $mangledName, $valueStorageName, $def, $doc) = @_;
    my %keys = $self->analyzeDoc($doc);
    %keys = $self->normalizeCommonKeys(%keys);
    %keys = $self->normalizeModelParameterKeys(%keys);
    $keys{'ulamParameterName'} = $uname;
    $keys{'valueStorageName'} = $valueStorageName;
    $keys{'mangledType'} = $mangledType;
    $keys{'valueDefault'} = $def;
    push @{$self->{'modelParameterList'}}, \%keys;
}

sub analyzeDoc {
    my ($self, $doc) = @_;
    my $name = $self->{'className'};
    if ($doc =~ m!\s*(/[*][*]\s*)?(.*?)\s*([*]/)?\s*$!s) {
        $doc = $2;
    } elsif ($doc eq "NONE") {
        $doc = "";
    } else {
        print STDERR "Unrecognized doc '$doc', treated as empty.\n";
        $doc = "";
    }

    my @pieces = split(/\\(?=[A-Za-z][A-Za-z])/,$doc); # uneaten lookahead!
    my $text = shift @pieces;
    $text = "" unless defined $text;

    my %keys;
    my ($brief, $detail) = ($text,"");
    if ($brief =~ /^([^.]+)[.]\s*(.*?)\s*$/s) {
        $brief = $1;
        $detail = $2;
    }
    $brief = "$name (no summary provided)" if $brief eq "";
    $detail = "(no details provided)" if $detail eq "";
    $keys{'brief'} = $brief;
    $keys{'detail'} = $detail;
    while (my $key = shift @pieces) {
        my $val = "";
        if ($key =~ /^([^ ]+)\s+(.*?)\s*$/) {
            $key = $1;
            $val = $2;
        }
        if (defined $keys{$key}) {
            $keys{$key} .= " ";
        } else {
            $keys{$key} = "";
        }
        $keys{$key} .= $val;
    }
    $keys{'name'} = $name;
    return %keys;
}

sub standardizeWhitespace {
    my ($self, $val) = @_;
    # Split on w/s runs including at least two newlines
    defined $val or die;
    my @pieces = split(/\s*\n\s*\n\s*/, $val);
    # Convert w/s runs within each piece to single space
    @pieces = map { $_ =~ s/\s+/ /g; $_ } @pieces;
    # Conjoin pieces with single blank lines between them
    return join("\n\n",@pieces);
}

sub normalizeCommonKeys {
    my ($self, %keys) = @_;

    ###
    # 'version' key analysis
    my $version = "0";
    if (defined $keys{'version'}) {
        $version = $1
            if $keys{'version'} =~ /^([0-9]+)$/;
    }
    if (length($version) > 1) {
        $version =~ s/^0+//;  # eat leading zeros
    }
    if (length($version) > 9) {
        $version = substr($version,-9);  # fuck you
    }
    $keys{'version'} = $version;

    return %keys;
}

sub normalizeClassKeys {
    my ($self, %keys) = @_;

    if (defined $keys{'colors'}) {
        print STDERR "'\\colors' key not accepted, ignored.  Use '\\color'\n";
        delete $keys{'colors'};
    }

    ###
    # 'color' key analysis

    my $w = "0xFFFFFFFF";
    if (!defined $keys{'color'}) {
        $keys{'color'} = $w;
    } else {
        my $c = $keys{'color'};
        if ($c =~ /^(dynamic|function)$/i) {
            print STDERR "Color keyword '$c' no longer accepted; using white ($w)\n";
            $c = $w;
        } elsif ($c =~ /^(0x|#)([0-9a-fA-F]{8})$/) {
            $c = "0x$2";
        } elsif  ($c =~ /^(0x|#)([0-9a-fA-F]{6})$/) {
            $c = "0xff$2";  # add alpha
        } elsif  ($c =~ /^(0x|#)([0-9a-fA-F])([0-9a-fA-F])([0-9a-fA-F])$/) {
            $c = "0xff$2$2$3$3$4$4"; # expand 12 bit color
        } else {
            print STDERR "Unrecognized color '$c', replaced with white ($w)\n";
            $c = $w;
        }
        $keys{'color'} = $c;
    }

    ###
    # 'symbol' key analysis

    my $symbol = "";
    if (defined $keys{'symbol'} && $keys{'symbol'} ne "") {
        $symbol = $keys{'symbol'};
    } else {
        $symbol = $keys{'name'};
    }
    if (length($symbol) > 2) {
        substr($symbol,2) = "";
    }
    substr($symbol,0,1) = uc(substr($symbol,0,1));
#    print STDERR "\nSYMV($symbol)\n";
    $keys{'symbol'} = $symbol;

    ###
    # 'symmetries' key analysis
    my %symkeys = (
        "0" => 1<<0, "1" => 1<<1, "2" => 1<<2, "3" => 1<<3,
        "4" => 1<<4, "5" => 1<<5, "6" => 1<<6, "7" => 1<<7,
        "all" => 0xff,
        "rotations" => 0x0f,
        "0L" => 1<<0, "90L" => 1<<1, "180L" => 1<<2, "270L" => 1<<3,
        "0R" => 1<<4, "90R" => 1<<5, "180R" => 1<<6, "270R" => 1<<7,
        "normal" => 1<<0,
        "none" => 1<<0,
        "flipx" => 1<<6, "flipy" => 1<<4, "flipxy" => 1<<2,
        );

    my @syms;
    my $symmetries;
    if (defined $keys{'symmetry'}) {
        $symmetries = $keys{'symmetry'};
        delete $keys{'symmetry'};
    }
    if (defined $keys{'symmetries'}) {
        warn "\\symmetries taking precedence over \\symmetry"
            if (defined $symmetries);
        $symmetries = $keys{'symmetries'};
    }
    $symmetries = "0" unless defined $symmetries;
    if ($symmetries =~ /^[0-7]+$/) {
        @syms = split(//,$symmetries);
    } else {
        @syms = split(/[\s,;]+/,$symmetries);
    }
    my $symbits = 0;
    for (my $i = 0; $i < scalar(@syms); ++$i) {
        my $k = $syms[$i];
        my $v = $symkeys{lc($k)};
        if (!defined $v) {
            warn "unknown symmetry '$k' ignored";
        } else {
            $symbits |= $v;
        }
    }
    if ($symbits == 0) {
        $symbits = 1<<0;
    }

    my $final = "";
    my $finalCount = 0;
    for (my $i = 0; $i < 8; ++$i) {
        if ($symbits&(1<<$i)) {
            ++$finalCount;
            $final .= ","
                if $final ne "";
            $final .= $i;
        }
    }
    $keys{'numsymmetries'} = $finalCount;
    $keys{'symmetries'} = $final;

    ###
    # 'radius' key analysis
    my $defaultRadius = 4;
    if (!defined $keys{'radius'}) {
        $keys{'radius'} = $defaultRadius;
    } else {
        my $r = $keys{'radius'};
        if ($r eq "default") {
            $r = $defaultRadius; 
        } elsif ($r !~ /^[0-4]$/) {
            print STDERR "\radius value '$r' not recognized; using $defaultRadius\n";
        }
    }


    ###
    # 'author' key analysis
    if (!defined $keys{'author'}) {
        $keys{'author'} = "--none specified--";
    }

    ###
    # 'copyright' key analysis
    if (!defined $keys{'copyright'}) {
        $keys{'copyright'} = "--none specified--";
    }

    ###
    # 'license' key analysis
    if (!defined $keys{'license'}) {
        $keys{'license'} = "--none specified--";
    }

    ###
    # 'placeable' key analysis
    if (!defined $keys{'placeable'}) {
        $keys{'placeable'} = "yes";
    }
    $keys{'placeable'} = lc($keys{'placeable'});
    $keys{'placeable'} = "no" if $keys{'placeable'} =~ /off|false|0/;
    $keys{'placeable'} = "yes" if $keys{'placeable'} =~ /on|true|1/;
    $keys{'placeable'} = "yes" unless $keys{'placeable'} eq "no";

    ###
    # 'diffusability' key analysis
    my $diffusability;
    my $inp = $keys{'diffusability'};
    $inp = $keys{'diffusable'} unless defined $inp;
    if (defined $inp && $inp =~ /^([0-9]+)$/) {
        $diffusability = $1;
        $diffusability = 100 if $diffusability > 100;
    }
    $keys{'diffusability'} = $diffusability;
    delete $keys{'diffusable'};

    return %keys;
}

sub normalizeDecimal {
    my ($self, $numstr) = @_;
    my $sign =  "";
    if ($numstr =~ s/^\s*[-+]//) {
        $sign = $1 if $1 eq "-";
    }
    $numstr =~ s/^0*(\d+)$/$1/;
    return "$sign$numstr";
}

sub normalizeModelParameterKeys {
    my ($self, %keys) = @_;

    ###
    # 'range' key analysis

    my $min = undef;
    my $max = undef;
    if (defined $keys{'range'}) {
        my $r = $keys{'range'};
        if ($r =~ /^\s*([+-]?\d+)\s*([.][.]|:|,)\s*([+-]?\d+)$/) {
            ($min, $max) = ($1,$3);
            $min = $self->normalizeDecimal($min);
            $max = $self->normalizeDecimal($max);
        }
    }
    $keys{'valueMin'} = $min;
    $keys{'valueMax'} = $max;

    return %keys;
}

sub escapeChar {
    my $char = shift;
    return "\\n" if $char eq "\n";
    return "\\\\" if $char eq "\\";
    return "\\a" if $char eq "\a";
    return "\\t" if $char eq "\t";
    return "\\\"" if $char eq "\"";
    return sprintf("\\x%02x",ord($char));
}

sub makeCString {
    my ($self, $string) = @_;
    $string =~ s/(["\\\n]|[^[:print:]])/escapeChar("$1")/ge;
    return '"'.$string.'"';
}

# Add all info, doc, parms etc, to $self first, somehow, then call
# this.  In particular, in self there will be
#
#  - $self->{'className'} : string name of class
#
#  - $self->{'classMangledName'} : string mangled name of class
#
#  - $self->{'classKeys'} : hashref to the keys from analyzeDoc on the
#    class info
#
#  - $self->{'parms'} : listref to the parms, each of which is a
#    hashref to the keys from analyzeDoc on the parm info, augmented
#    with other stuff we got out of the ULAM INFO: line.
sub generateUlamElementInfo {
    my ($self) = @_;
    my %classKeys = %{$self->{'classKeys'}};
    my @parms = @{$self->{'modelParameterList'}};

    my $infocode = "";

    my $stamp = gmtime(time());
    my $cname = $self->makeCString($self->{'className'});

    my $csym = $self->makeCString($classKeys{'symbol'});
    my $csum = $self->makeCString($self->standardizeWhitespace($classKeys{'brief'}));
    my $cdet = $self->makeCString($self->standardizeWhitespace($classKeys{'detail'}));

    my $movfunc = "";
    my $pct = $classKeys{'diffusability'}; # XXX KEEP THIS? SRSLY?
    if (defined $pct) {
        $movfunc = <<EOF;
    virtual const u32 GetPercentDiffusability() const
    {
      return $pct;
    }
EOF
    }

    my $author = $self->makeCString($classKeys{'author'});
    my $copyright = $self->makeCString($classKeys{'copyright'});
    my $license = $self->makeCString($classKeys{'license'});
    my $radius = $classKeys{'radius'};
    my $cver = $classKeys{'version'};
    my $cplaceable = "true";
    $cplaceable = "false" if $classKeys{'placeable'} eq "no";

    my $color = $classKeys{'color'};
    my $body = "";
    $body .= "return $color;";

    my @syms = split(/,/,$classKeys{'symmetries'});
    my $csyc = scalar(@syms);
    my $symbody = "";
    if ($csyc == 1) {
        $symbody .= "return $syms[0];";
    } else {
        $symbody .= "Random & r = const_cast<UlamContext<EC> &>(uc).GetRandom();\n";
        if ($csyc == 8) {
            $symbody .= "      return r.Create($csyc);";
        } else {
            $symbody .= "      switch (r.Create($csyc)) {\n";
            $symbody .= "        default: /* not reached */";
            for (my $i = 0; $i < $csyc; ++$i) {
                $symbody .= "\n        case $i: return $syms[$i];"
            }
            $symbody .= "\n      }";
        }
    }
    my $mangledName = $self->{'classMangledName'};
    die unless defined $mangledName;
    my $file = $self->{'classFile'};
    die unless defined $file;
    my $structName = "UlamElementInfoFor$mangledName";
    $infocode .= <<EOF;
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

    UlamElement<EC> & m_ulamElement;
    $structName(UlamElement<EC> & ue)
        : m_ulamElement(ue)
    {
      AddModelParameters();
    }

    $structName() { }
    const char * GetName() const { return $cname; }
    const char * GetSymbol() const { return $csym; }
    const char * GetSummary() const { return $csum; }
    const char * GetDetails() const { return $cdet; }
    const char * GetAuthor() const { return $author; }
    const char * GetCopyright() const { return $copyright; }
    const char * GetLicense() const { return $license; }
    bool GetPlaceable() const { return $cplaceable; }
    const u32 GetVersion() const { return $cver; }
$movfunc
    const u32 GetElementColor() const { $body }
    const u32 GetEventWindowBoundary() const { return $radius + 1; }
    const u32 GetSymmetry(const UlamContext<EC>& uc) const {
      $symbody
    }
EOF
    my $modelParameterCount = scalar(@{$self->{'modelParameterList'}});

    $infocode .= <<EOF;

    void AddModelParameters()
    {
EOF
    for (my $index = 0; $index < $modelParameterCount; ++$index) {
        my $href = $self->{'modelParameterList'}->[$index];
#        $self->dumpHash($href,0,*STDERR);
        my %keys = %{$href};
        my $mtype = $keys{'mangledType'};
        $mtype =~ /([bituvy])$/ or die "Unrecognized type '$mtype'";
        my $ptype = $1;
        if ($ptype eq "i" || $ptype eq "u" || $ptype eq "y") {
            $self->GenParmSimple($href, \$infocode,$ptype);
        } elsif ($ptype eq "b") {
            $self->GenParmBool($href, \$infocode,$ptype);
        } else {
            die "Unimplemented model parameter type '$ptype'";
        }
    }
    $infocode .= <<EOF;
    }
EOF
    $infocode .= <<EOF;
  };
} // MFM

EOF
  return ($structName,$infocode);
}

sub GenParmSimple {
    my ($self, $href, $sink, $type) = @_;
    my $mangledename = $self->{'classMangledName'};
    my $suf;
    my $ctype = "u32";
    if ($type eq "u") {
        $suf = "U32";
    } elsif ($type eq "i") {
        $suf = "S32";
        $ctype = "s32";
    } elsif ($type eq "y") {
        $suf = "Unary";
    } else {
        die "Unrecognized '$type'";
    }
    my %keys = %{$href};
    my $vsn = $keys{'valueStorageName'};
    my $mantype = $keys{'mangledType'};
    my $uname = $keys{'ulamParameterName'};
    my $cbrief = $self->makeCString($self->standardizeWhitespace($keys{'brief'}));
    my $cdetail = $self->makeCString($self->standardizeWhitespace($keys{'detail'}));
    my $unit = $keys{'unit'};
    my $cunit = '""';
    $cunit = $self->makeCString($self->standardizeWhitespace($unit))
        if defined $unit;

    my $def = $keys{'valueDefault'};
    my $decldef = "//no default";
    my $pdecldef = "0";
    if (defined $def) {
        if ($type eq "y") {
            $decldef = "u32 _def = (u32) _Unary32ToUnsigned32($def,32,32);";
        } else {
            $decldef = "$ctype _def = ($ctype) $def;";
        } 
        $pdecldef = "&_def";
    }

    my $max = $keys{'valueMax'};
    my $declmax = "//no max";
    my $pdeclmax = "0";
    if (defined $max) {
        $declmax = "$ctype _max = ($ctype) $max;";
        $pdeclmax = "&_max";
    }

    my $min = $keys{'valueMin'};
    my $declmin = "//no min";
    my $pdeclmin = "0";
    if (defined $min) {
        $declmin = "$ctype _min = ($ctype) $min;";
        $pdeclmin = "&_min";
    }

    $$sink .= <<EOF;
    {
        $declmin
        $decldef
        $declmax
        static UlamTypeInfoModelParameter$suf<EC> parm(
            m_ulamElement,
            "$mantype",
            "$uname",
            $cbrief,
            $cdetail,
            $pdeclmin,
            $pdecldef,
            $pdeclmax,
            $cunit);
        //ctor registers itself on element..
        $mangledename<EC>::THE_INSTANCE.$vsn.init(parm.GetAtom());
    }
EOF

}

sub GenParmBool {
    my ($self, $href, $sink, $type) = @_;
    my $mangledename = $self->{'classMangledName'};
    my $suf;
    if ($type eq "b") {
        $suf = "Bool";
    } else {
        die "Unrecognized '$type'";
    }
    my %keys = %{$href};
    my $vsn = $keys{'valueStorageName'};
    my $mantype = $keys{'mangledType'};
    my $uname = $keys{'ulamParameterName'};
    my $cbrief = $self->makeCString($self->standardizeWhitespace($keys{'brief'}));
    my $cdetail = $self->makeCString($self->standardizeWhitespace($keys{'detail'}));
#    my $cunit = $self->makeCString($self->standardizeWhitespace($keys{'unit'}));

    my $def = $keys{'valueDefault'};
    $def = "false" unless defined $def;

    $$sink .= <<EOF;
    {
        static UlamTypeInfoModelParameterBool<EC> parm(
            m_ulamElement,
            "$mantype",
            "$uname",
            $cbrief,
            $cdetail,
            $def);
        $mangledename<EC>::THE_INSTANCE.$vsn.init(parm.GetAtom());
    }
EOF

}

1
