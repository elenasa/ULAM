package ULAMScanner;
use strict;
use warnings;
use diagnostics;
use Carp;

our $VERSION = 1.001;

sub new {
    my $class = shift;
    my $self = {
        "input" => "",   # Remaining unprocessed input
        "toklist" => [], # all tokens
        "scmtidx" => [], # indices of structured comments
        "clsidx" => [],  # indices of class headers
        "stridx" => [],  # indices of quoted strings
    };
    bless $self, $class;
    return $self;
}

sub load {
    my ($self,$handle) = @_;
    my $all = $self->{'input'};
    while (<$handle>) { $all .= $_; }
    $self->{'input'} = $all;
}

sub scan {
    my $self = shift;
    my $tokidx;
    for ($tokidx = 0; $self->{'input'} ne ""; ++$tokidx) {
        my ($type, $match) = $self->scanNextItem();
#        print STDERR "SCAN($type,$match)\n";
        push @{$self->{'toklist'}}, [$type, $match];
        push @{$self->{'scmtidx'}}, $tokidx
            if $type eq 'scmt';
        push @{$self->{'clsidx'}}, $tokidx
            if $type eq 'hdr';
        push @{$self->{'stridx'}}, $tokidx
            if $type eq 'str';
    }
}

sub print {
    my ($self, $handle) = @_;
    foreach my $tok (@{$self->{'toklist'}}) {
        print $handle $tok->[1];
    }
}

sub getClassDoc {
    my ($self, $class) = @_;
    for my $idx (@{$self->{'clsidx'}}) {
        next if $idx == 0;
        my $val = $self->{'toklist'}->[$idx]->[1];
        my $prevtok = $self->{'toklist'}->[$idx-1];
        next unless $prevtok->[0] eq 'scmt';
        next unless $val =~ /^(element|quark|union)\s+([A-Z][A-Za-z0-9]*)$/;
        my $className = $2;
        return $prevtok->[1] if $className eq $class;
    }
    return "";
}

sub analyzeDoc {
    my ($self, $name, $doc) = @_;
    $doc = $2
        if $doc =~ m!\s*(/[*][*]\s*)?(.*?)\s*([*]/)?\s*$!s;
    my @pieces = split(/\\(?=[A-Za-z][A-Za-z])/,$doc); # uneaten lookahead!
    my $text = shift @pieces;
    $text = "" unless defined $text;

    my %keys;
    my ($brief, $detail) = ($text,"");
    if ($brief =~ /^([^.]+[.])\s*(.*?)\s*$/s) {
        $brief = $1;
        $detail = $2;
    }
    $brief = "$name (no summary provided)" if $brief eq "";
    $detail = "$name (no details provided)" if $detail eq "";
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
    my @pieces = split(/\s*\n\s*\n\s*/, $val);
    # Convert w/s runs within each piece to single space
    @pieces = map { $_ =~ s/\s+/ /g; $_ } @pieces;
    # Conjoin pieces with single blank lines between them
    return join("\n\n",@pieces);
}

sub normalizeKeys {
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

sub scanNextItem {
    my $self = shift;
    my $input = $self->{'input'};
    (my $type, my $match, $input) = scanNextItemLL($input);
    $self->{'input'} = $input;
    return ($type, $match);
}

sub scanNextItemLL {
    my $input = shift;

    # Match ws and non-structured comments
    {
        my $match = "";
        my $again = 1;

        while ($again) {
            $again = 0;
            if ($input =~ s!^(\s+|//[^\n]*?\n)!!s) {
                $match .= $1;
                $again = 1;
            }
            if ($input =~ s!^(/[*][^*].*?[*]/)!!s) {
                $match .= $1;
                $again = 1;
            }
        }
        return ('ws',$match,$input)
            if $match ne "";
    }

    # Match structured comment plus ws
    if ($input =~ s!^(/[*][*].*?[*]/\s*)!!s) {
        return ('scmt', $1, $input);
    }

    # Match ulam or use + ws
    if ($input =~ s!^((ulam|use)\s+[^;]*?;\s*)!!s) {
        return ('pre', $1, $input);
    }

    # Match element parameters + ws
    if ($input =~ s!^((element)\s+[^{;]*;\s*)!!s) {
        return ('epm', $1, $input);
    }

    # Match quark, union, or element header with possible template parameters (ignored)
    if ($input =~ s!^((quark|union|element)\s+([A-Z][A-Za-z0-9]*))[^{]*!!s) {
        return ('hdr', $1, $input);
    }

    # Match two char operators + ws
    if ($input =~ s!^(&&|\|\|)(\s*)!!s) {
        return ($1, "$1$2", $input);
    }

    # Match single brackets, operators, and delimiters + ws
    if ($input =~ s!^([\[\](){};,=.<>~\!/*&|+-])(\s*)!!s) {
        return ($1, "$1$2", $input);
    }

    # Match strings
    if ($input =~ s!^("(?:\\"|\\\\|[^\n])*?")!!s) {
        return ('str', $1, $input);
    }

    # Match number
    if ($input =~ s!^([0-9]+)!!s) {
        return ('number', $1, $input);
    }

    # Match type name, with possible template arguments (assuming no nested parens.. true?)
    if ($input =~ s!^([A-Z][A-Za-z0-9_]*(\([^\)]+\))?)!!s) {
        return ('type', $1, $input);
    }

    # Match non-type name (and keywords)
    if ($input =~ s!^([a-z][A-Za-z0-9_]*)!!s) {
        return ('name', $1, $input);
    }

    # Match arbitrary leftover
    return ('rest',$input,"");
}
1
__END__

for (my $tokidx = 0; $all ne ""; ++$tokidx) {
    (my $type, my $match, $all) = nextItem($all);
    push @toklist, [$type, $match];
    push @scmtidx, $tokidx
        if $type eq 'scmt';
    push @stridx, $tokidx
        if $type eq 'str';
}

sub analyzeElementScmt {
    my ($scmt, $elt) = @_;
    my $eltname;
    if ($elt =~ /^element\s+([^\s]+)\s*$/) {
        $eltname = $1;
    } else {
        print STDERR "Unrecognized '$elt', ignored\n";
        return;
    }

    print "AES(($scmt,$eltname))\n";
}

foreach my $i (@scmtidx) {
    my $stok = $toklist[$i];
    my $ntok = $toklist[$i+1];
    last unless defined $ntok;

    if ($ntok->[0] eq 'hdr' && $ntok->[1] =~ /^element/) {
        analyzeElementScmt($stok->[1],$ntok->[1]);
    } else {
#        $toklist[$i+1]->[1] = "SCMT/UNREC<<".$toklist[$i+1]->[1].">>";
    }
}

foreach my $tok (@toklist) {
    print $tok->[1];
}

if (0) {
while ($all ne "") {
    if (length($type) == 1) {
        print "<${type}${match}>";
    } elsif ($match ne "") {
        print "<${type}>$match";
    } else {
        print "<${type}>";
    }
}
}

1;

__END__
last line of the module needs to be true;
last line of the _file_ need not be true:
0;
