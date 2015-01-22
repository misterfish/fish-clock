#!/usr/bin/env perl

# Not used.

package Fish::Clock::Resolution;

BEGIN {
    use File::Basename;
    push @INC, dirname $0;
}

use 5.10.0;

#use Moose;
use Moo;
use MooX::Types::MooseLike::Base ':all';    

use Fish::Clock::Utility;

$| = 1;

my $W = 'Fish::Res -- ';

has x => (
    is  => 'rw',
    #isa => 'Int',
    isa => Int,
    writer => 'set_x',
);
has y => (
    is  => 'rw',
    #isa => 'Int',
    isa => Int,
    writer => 'set_y',
);

sub BUILD {
    my ($self) = @_;
    my $cmd = 'xrandr';
    if (! sys_ok qq, which "$cmd" , ) {
        war $W, "No", R $cmd, 'found in path';
        return;
    }
    my ($l, $code) = sysl $cmd, { die => 0 };
    if ($code) {
        war $W, "Error with cmd:", Y $cmd, R $code;
        return;
    }
    # star = current, plus = preferred
    #    1920x1080      60.0*+   60.0
    #
    my $found;
    for (list $l) {
        if ( / ^ \s* (\d+)x(\d+) \s+ \d+ (\.\d+)? \* /mx ) {
            if ($found) {
                war $W, "Multiple screens/displays found; xrandr needs params.";
            }
            $found = 1;
            $self->set_x($1);
            $self->set_y($2);
        }
    }

}

1;
