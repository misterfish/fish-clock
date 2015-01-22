#!/usr/bin/env perl 

package Fish::Clock::Conf;

BEGIN {
    use File::Basename;
    push @INC, dirname $0;

    use Exporter;

    @ISA = 'Exporter';
    @EXPORT = qw, c cr cr_list ,;
}

use 5.10.0;

use strict;
use warnings;

use Cwd 'realpath';
use File::stat;
use Config::IniFiles;

use Fish::Utility;
use Fish::Utility_l qw, chompp ,;
use Fish::Class 'o';

sub c(_);
sub cr(_);

my $DEFAULT = '_default';

# - - - 

my $g = o(
    hostname => chompp(sys 'hostname'),
    conf_files => [],
    conf_files_rejected => [],
    cfg => undef,

    cache1 => {},
    cache2 => {},

    c => {},
);

# - - - 

# Just search for 'user'
sub pre_init { shift if __PACKAGE__ eq $_[0];
    my ($file) = @_;
    my $cfg = get_cfg($file);
    my $user = $cfg->val($DEFAULT, 'user') 
        or error "Need", CY 'user', "in main config";
    return strip $user;
}

sub init { shift if __PACKAGE__ eq $_[0];

    my ($opts) = @_;
    $opts //= {};

    my $required_ary = $opts->{required} // [];

    my $cf = $opts->{conf_files} // die "Need opt:", BR 'conf_files';
    die unless ref $cf eq 'ARRAY';

    my ($ok, $not_ok) = check_files(@$cf);
    my @ok_conf_files = @$ok;
    my @not_ok_conf_files = @$not_ok;

    $g->conf_files(\@ok_conf_files);
    $g->conf_files_rejected(\@not_ok_conf_files);

    read_config();

    if ($opts->{dump}) {
        $g->cfg->OutputConfigToFileHandle(*STDOUT);
        return;
    }

    ref $required_ary eq 'ARRAY' or die 'invalid arg';
    cr for @$required_ary; # check and throw away

    main::timeout( 500, sub {
        read_config();
        1
    });
}

sub read_config {
    state $stamp;
    %$_ = () for $g->cache1, $g->cache2;
    my $cf = $g->conf_files;
    
    my $changed;
    for (@$cf) {
        # In the instant that it's being saved, file is not there.
        my $stat = stat $_ or return;

        next if defined $stamp and $stat->mtime <= $stamp;
        $stamp = $stat->mtime;
        $changed = 1;
    }

    return unless $changed;

    info 'Updating config.';

    my @c = @$cf;
    #my ($cfg_base, $cfg_overlay);
    my $cfg = get_cfg(shift @c);
    for (@c) {
        my $cfg_overlay = get_cfg($_);
        my $cfg_mixed = get_cfg($_, { base => $cfg });
        $cfg = $cfg_mixed;
    }

    $g->cfg($cfg);
}

sub get_cfg {
    my ($cf, $opts) = @_;
    $opts //= {};
    my @overlay;
    @overlay = (-import => $opts->{base}) if $opts->{base};

    my $cfg = Config::IniFiles->new( 
        -file => $cf,
        -default => $DEFAULT,
        -handle_trailing_comment => 1,
        @overlay,
    ) 
        or error "Couldn't read config file", R $cf;
    return $cfg;
}

sub _c {
    my ($k, $required) = @_;
    $k // die;

    state $cache = $g->cache1;
    if (my $v = $cache->{$k}) {
        return $v;
    }
    my $v = $g->cfg->val($g->hostname, $k);
    if ($required and not defined $v) {
        my @ok_cf = list $g->conf_files;
        my @not_ok_cf = list $g->conf_files_rejected;
        @ok_cf = map { G $_ } @ok_cf;
        @not_ok_cf = map { BR $_ } @not_ok_cf;
        my @all_cf = (@ok_cf, @not_ok_cf);
        my $s = @all_cf > 1 ? 'conf files were' : "conf file is";
        error sprintf "Config key %s is required (%s %s)", R $k, $s, join ', ', @all_cf;
    }
    # value given multiple times should override, not get a \n in the middle.
    $v = (split /\n/, $v)[-1] if $v; 
    strip_s $v;
    $cache->{$k} = $v;

    $v
}

sub c(_) {
    return _c(shift, 0);
}

# config, required=1
sub cr(_) {
    return _c(shift, 1);
}

# config, required=1, split list
sub cr_list($) {
    my ($k) = @_;
    state $cache = $g->cache2;
    return $cache->{$k} // 
        ($cache->{$k} = [split / \s* , \s* /x, cr $k]);
}

sub check_files {
    my @files = @_;
    my @ok;
    my @not_ok;
    for my $f (@files) {
        if (not -f $f) { 
            push @not_ok, $f;
            next;
        }
        if (not -r $f) {
            war "Not readable:", BR $f;
            push @not_ok, $f;
            next;
        }
        push @ok, $f;
    }
    @ok or error "Couldn't find config in", \@files;
    return \@ok, \@not_ok;
}

1;
