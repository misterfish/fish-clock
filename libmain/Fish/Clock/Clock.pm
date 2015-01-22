package Fish::Clock::Clock;

use 5.18.0;

BEGIN {
    use File::Basename;
    push @INC, dirname $0;
}

$| = 1;

use Moo;
use MooX::Types::MooseLike::Base ':all';    

use Date::Calc 'Add_Delta_DHMS';
use List::Util 'max', 'min';
use Math::Trig ':pi';
use X11::Aosd ':all';
use Time::HiRes qw, time sleep ,;

use Fish::Class 'o';
use Fish::Utility;
use Fish::Utility_l qw, pairwiser ,;

use Fish::Clock::Conf 'c', 'cr', 'cr_list';
use fish_clock_xs;

# XX
use constant DEBUG => 0;

use constant DEBUG_TIME => 1;

# jittery when 1. but like this it takes more mouse space.
use constant RESIZE_BOUNDS_ON_ZOOM => 0;

use constant RANDOM_TIME_ON_ZOOM_OUT => 0; # really random

# res -> radius_out
# toaster, xres = 1920, radius_out => 600
# venkel, xres = 1024, radius_out => 300
# 1920a + b = 600
# 1024a + b = 300
# a = 300 / 894 = .34
# b = -43

# Try to use g for config stuff, self for changey stuff. XX
my $g = o(
    radius_out => undef,
    num_frames_out => undef,
    num_frames_in => undef,
    scale_radius_in => undef,
    scale_radius_out => undef,
    radius => undef,

    # should stay global
    transparency => undef,

    hand_length_major => undef,
    hand_length_minor => undef,
    tick_length_major => undef,
    tick_length_minor => undef,

    delta_theta_ticks => undef,
    delta_theta_hands => undef,

    num_ticks           => undef,
    major_ticks_every   => undef,

    hour_fix            => undef,
    min_fix             => undef,

    stroke_width_circle => undef,
    stroke_width_ticks => undef,
    stroke_width_hands => undef,
    circle_2_x_shift => undef,
    circle_2_delta_radius => undef,

    random_min => undef,

    do_wave => 0,
    radius_min => undef,
    stretch_factor => undef,
    radius_calc => [],
);

has hour => (
    is  => 'ro',
    #isa => 'Ref',
    isa => Ref,
);
# s because min exists
has mins => (
    is  => 'ro',
    #isa => 'Ref',
    isa => Ref,
);
has sec => (
    is  => 'ro',
    #isa => 'Ref',
    isa => Ref,
);

has xres => (
    is  => 'ro',
    isa => Int,
);

# General property, whether to do any filling at all.
has fill => (
    is => 'rw',
    #isa => 'Bool',
    isa => Bool,
);

has _aosd => (
    is  => 'rw',
    #isa => 'X11::Aosd',
);
has _radius => (
    is  => 'rw',
    #isa => 'Num',
    isa => Num,
);
has _width => (
    is  => 'rw',
    #isa => 'Num',
    isa => Num,
);
has _height => (
    is  => 'rw',
    #isa => 'Num',
    isa => Num,
);
has _transparency => (
    is  => 'rw',
    #isa => 'Num',
    isa => Num,
);

# in = -1, out = 1, no = 0
has _zooming => (
    is  => 'rw',
    #isa => 'Int',
    isa => Int,
);
has _hour_override => (
    is => 'rw',
);

has _min_override => (
    is => 'rw',
);

has _delta_min_for_random_zoom_in => (
    is => 'rw',
    #isa => 'Num',
    isa => Num,
);

# render loop

# whether to fill during this render frame.
has _do_fill => (
    is => 'rw',
    #isa => 'Bool',
    isa => Bool,
);
has _color_circle_1 => (
    is => 'rw',
    #isa => 'ArrayRef',
    isa => ArrayRef,
);
has _color_circle_2 => (
    is => 'rw',
    #isa => 'ArrayRef',
    isa => ArrayRef,
);

sub BUILD {
    my ($self) = @_;

    info 'n frames in', $g->num_frames_in if DEBUG;
    info 'n frames out', $g->num_frames_out if DEBUG;

    # config XX
    if (0) {
        my $r = Fish::Clock::Resolution->new;
        if ( my $xres = $r->x ) {
            $g->radius_out( $xres * .34 - 43 ); # config XX
        }
        else {
            warn "Couldn't get xres.";
            $g->radius_out(300);
        }
    }
    $g->radius_out( $self->xres * .34 - 43 ); # config XX
    $self->_radius($g->radius_out);

    # sets _aosd
    $self->init;
}

sub update {
    my ($self) = @_;
    my $aosd = $self->_aosd or warn, return;

    # so it stays centered, not necessary
    $self->update_boundaries if RESIZE_BOUNDS_ON_ZOOM;

    # Trigger renderer.
    # render is (maybe) faster than update, but the very last frame won't be
    # shown.

    #$aosd->render;
    $aosd->update;
}

sub show {
    my ($self) = @_;

    #main::bench_start('show-clock.1');
    $self->update_config;

    my $aosd = $self->_aosd;

    $self->_zooming(-1);

    $self->_do_fill($self->fill ? cr 'fill-on-zoom-in' : 0);

    #main::bench_end('show-clock.1');
    #main::bench_start 'show-clock.2';

    $aosd->show;

    #main::bench_end 'show-clock.2';

    my $r;
    my $tr = .001;

    my $num_frames = $g->num_frames_in;

    my $dhl1 = $g->hand_length_major / $num_frames;
    my $dhl2 = $g->hand_length_minor / $num_frames;

    my $hour = $self->hour;
    my $min = $self->mins;
    my $sec = $self->sec;

    my $_r = -1;

    my $_i = -1;
    main::timeout(cr 'animate-interval', sub {
        $_i++;

        #main::bench_start 'show-clock.3.1';

        # Start at random amount below actual, then go up to actual evenly.
        if (my $rm = $g->random_min) {

            my ($hr_new, $min_new);
            if ($_i == 0) {
                (undef, undef, undef, $hr_new, $min_new) = Add_Delta_DHMS(10, 10, 10, $$hour, $$min, 0, 0, 0, -1 * $rm, 0);
                # will overshoot if very small random_min given.
                my $dm = max 1, $rm / $num_frames;

                $self->_delta_min_for_random_zoom_in($dm);
                $self->_hour_override($hr_new);
                $self->_min_override($min_new);
            }
            else {
                (undef, undef, undef, $hr_new, $min_new) = Add_Delta_DHMS(10, 10, 10, $self->_hour_override, $self->_min_override, 0, 0, 0, $self->_delta_min_for_random_zoom_in, 0);
                $self->_hour_override($hr_new);
                $self->_min_override($min_new);
            }
        }

        $tr += $g->transparency / $num_frames;

        my $rc = $g->radius_calc;

        $r = $g->do_wave ? 
            $rc->[$_i] :
            $g->radius_out / ($g->scale_radius_in ** $_i);

        my $_tr = $tr;
        $self->set_radius($r);
        $self->_transparency($_tr);

        #main::bench_end 'show-clock.3.1';

        #main::bench_start 'show-clock.3.2';

        $self->update;

        #main::bench_end 'show-clock.3.2';

        if ($_i == $num_frames) {
            $self->_zooming(0);
            $self->show_final;
            return 0;
        }
        return 1;
    });
}

sub show_final {
    my ($self) = @_;

    #main::bench_start 'show-clock.4';

    $self->_hour_override(undef);
    $self->_min_override(undef);

    if (my $hr = $g->hour_fix) {
        my $min = $g->min_fix;
        $self->_hour_override($hr);
        $self->_min_override($min);
    }

    if ($self->fill) {
        $self->_do_fill(1);
        $self->_color_circle_1(cr_list 'color-circle-1-filled');
        $self->_color_circle_2(cr_list 'color-circle-2-filled');
    }
    else {
        $self->_do_fill(0);
        $self->_color_circle_1(cr_list 'color-circle-1');
        $self->_color_circle_2(cr_list 'color-circle-2');
    }

    $self->update;
        
    #main::bench_end 'show-clock.4';
}

sub hide {
    my ($self) = @_;

    my $aosd = $self->_aosd;

    $self->_zooming(1);

    my $tr = $self->_transparency;
    my $tr_step = (.001 - $g->transparency) / $g->num_frames_out;

    if ($self->fill) {
        $self->_do_fill(cr 'fill-on-zoom-out');
        $self->_color_circle_1(cr_list 'color-circle-1-filled');
        $self->_color_circle_2(cr_list 'color-circle-2-filled');
    }
    else {
        $self->_do_fill(0);
        $self->_color_circle_1(cr_list 'color-circle-1');
        $self->_color_circle_2(cr_list 'color-circle-2');
    }

    my $r = $self->_radius;

    my $ro = $g->radius_out;
    while ($r < $ro) {

        if (RANDOM_TIME_ON_ZOOM_OUT) {
            $self->_hour_override(1 + int(12 * rand));
            $self->_min_override(int(60 * rand));
        }

        $tr += $tr_step;
        $r *= $g->scale_radius_out;

        $self->_transparency($tr);
        $self->set_radius($r);

        $self->update;
    }
    
    if (RANDOM_TIME_ON_ZOOM_OUT) {
        $self->_hour_override(undef);
        $self->_min_override(undef);
    }

    $self->_zooming(0);

    $aosd->hide;
    $self->update;

    $self->set_radius($g->radius_out);

    return 1;
}

sub update_boundaries {
    my ($self) = @_;

    my $aosd = $self->_aosd or return;

    $aosd->set_position_with_offset(
      # x
      COORDINATE_CENTER,
      # y
      COORDINATE_CENTER,
      # w, h
      $self->_width, $self->_height, 
      # offset x, offset y
      0, 0,
    );

}

sub init {
    my ($self) = @_;

    my $aosd = X11::Aosd->new;
    $self->_aosd($aosd);

    $self->update_config;

    $aosd->set_transparency(TRANSPARENCY_COMPOSITE);

    $self->update_boundaries;

    # Doesn't work.
    $aosd->set_hide_upon_mouse_event(1);

    # These don't change (currently).
    my $height = $self->_height;
    my $width = $self->_width;

    my $hour = $self->hour;
    my $min = $self->mins;
    my $sec = $self->sec;

    $aosd->set_renderer( sub { 
        my ($cr) = @_;

        #main::bench_start 'renderer-config';

        my %a = (
            hour => $self->_hour_override // $$hour, 
            min => $self->_min_override // $$min, 
            sec => $$sec, 
            height => $height, 
            width => $width,
            (pairwiser { $a => $b } [qw, radius do_fill,], [$self->_radius, $self->_do_fill]),
            (pairwiser { $a => $b } [qw, transparency zooming,], [$self->_transparency, $self->_zooming]),
            (pairwiser { $a => $b } [qw, r1 g1 b1,], $self->_color_circle_1),
            (pairwiser { $a => $b } [qw, r2 g2 b2,], $self->_color_circle_2),
        );
        #Devel::Peek::Dump($cr);
        fish_clock_xs::render_config(\%a);

        #main::bench_end 'renderer-config';
        #main::bench_start 'renderer-render';

        fish_clock_xs::render_render($cr);

        #main::bench_end 'renderer-render';

    });
}
        
# also set w and h
sub set_radius {
    my ($self, $r) = @_;
    $self->_radius($r);

    my $fudge = 50;

    $self->_height($r * 2 + $fudge) if RESIZE_BOUNDS_ON_ZOOM;
    $self->_width($r * 2 + $fudge) if RESIZE_BOUNDS_ON_ZOOM;
}

sub get_fancy_r {
    # $t = parameter
    # $n = num frames
    my ($t, $n) = @_;

    my $min = $g->radius_min;
    my $sf = $g->stretch_factor;

    # t = n => sf term = 1
    my $sf_adj = 1 - sqrt(1 * $sf);

    max $min,
        # y-adj
        $g->radius + 
        # wave
        ($g->radius_out - $g->radius) * cos (2*pi*$t/$n * 
        # wave stretcher
            (($t/$n * $sf)**2 + $sf_adj)
        ) *
        # envelope
        (- 1.0 / $n * $t + 1)

}

sub update_config {
    my ($self) = @_;

    # radius * (scale_out ^ x) = radius_out
    # radius_out / (scale_in ^ x) = radius

    $g->random_min(c 'random-min');

    $g->stretch_factor(cr 'stretch-factor');
    $g->radius_min(cr 'radius-min');
    $g->do_wave(cr 'do-wave');
    $g->transparency(cr 'opacity');

    $self->_height(cr 'height');
    $self->_width(cr 'width');

    $g->hand_length_major(cr 'hand-length-major');
    $g->hand_length_minor(cr 'hand-length-minor');
    $g->tick_length_major(cr 'tick-length-major');
    $g->tick_length_minor(cr 'tick-length-minor');
    $g->delta_theta_ticks(cr 'delta-theta-ticks');
    $g->delta_theta_hands(cr 'delta-theta-hands');

    $g->stroke_width_circle(cr 'stroke-width-circle');
    $g->stroke_width_ticks(cr 'stroke-width-ticks');
    $g->stroke_width_hands(cr 'stroke-width-hands');

    $g->circle_2_x_shift(cr 'circle-2-x-shift');
    $g->circle_2_delta_radius(cr 'circle-2-delta-radius');

    $g->num_ticks(cr 'num-ticks');
    $g->major_ticks_every(cr 'major-ticks-every');

    my $num_frames_in = cr 'num-frames-in';
    my $num_frames_out = cr 'num-frames-out';
    my $radius = cr 'radius';
    $g->radius($radius);
    $g->scale_radius_in( ($g->radius_out / $radius) ** (1 / $num_frames_in) );
    $g->scale_radius_out( ($g->radius_out / $radius) ** (1 / $num_frames_out) );
    $g->num_frames_in($num_frames_in);
    $g->num_frames_out($num_frames_out);

    $g->hour_fix(c 'hour-fix');
    $g->min_fix(c 'min-fix');

    if ($self->fill) {
        $self->_color_circle_1(cr_list 'color-circle-1-filled');
        $self->_color_circle_2(cr_list 'color-circle-2-filled');
    }
    else {
        $self->_color_circle_1(cr_list 'color-circle-1');
        $self->_color_circle_2(cr_list 'color-circle-2');
    }
    # fill $g->radius_calc
    calc_fancy_r($num_frames_in);

    $self->init_renderer;
}

sub calc_fancy_r {
    my ($num_frames) = @_;
    for my $t (0 .. $num_frames) {
        $g->radius_calc->[$t] = get_fancy_r($t, $num_frames);
    }
}

sub init_renderer {
    my ($self) = @_;
    my $fill = $self->fill;
    my %a = (
        (pairwiser { $a => $b } [qw, th_circle th_ticks th_hands,], [$g->stroke_width_circle, $g->stroke_width_ticks, $g->stroke_width_hands]),
        (pairwiser { $a => $b } [qw, fr fg fb,], cr_list 'color-fill'),
        (pairwiser { $a => $b } [qw, t1r t1g t1b ,], $fill ? cr_list 'color-ticks-filled-1' : cr_list 'color-ticks-1'),
        (pairwiser { $a => $b } [qw, t2r t2g t2b ,], $fill ? cr_list 'color-ticks-filled-2' : cr_list 'color-ticks-2'),
        (pairwiser { $a => $b } [qw, h1r h1g h1b ,], $fill ? cr_list 'color-hands-1-filled' : cr_list 'color-hands-1'),
        (pairwiser { $a => $b } [qw, h2r h2g h2b ,], $fill ? cr_list 'color-hands-2-filled' : cr_list 'color-hands-2'),
        circle_2_x_shift => $g->circle_2_x_shift,
        circle_2_delta_radius => $g->circle_2_delta_radius,
        num_ticks => $g->num_ticks,
        major_ticks_every => $g->major_ticks_every,
        (pairwiser { $a => $b } [qw, tl_major tl_minor ,], [$g->tick_length_major, $g->tick_length_minor]),
        (pairwiser { $a => $b } [qw, hl_major hl_minor ,], [$g->hand_length_major, $g->hand_length_minor]),
        (pairwiser { $a => $b } [qw, dt_ticks dt_hands ,], [$g->delta_theta_ticks, $g->delta_theta_hands]),
    );

    fish_clock_xs::render_init(\%a);
}

sub log10 { log(shift)/log(10) }


__PACKAGE__->meta->make_immutable;

1;
