use ExtUtils::MakeMaker;
use List::MoreUtils 'uniq';

my @inc = qw,
    -I.
,;
my $inc = join ' ', uniq @inc;

# --- see lib/ExtUtils/MakeMaker.pm for details of how to influence the
# contents of the Makefile that is written.

WriteMakefile(
   NAME         => 'fish_clock_xs',
   VERSION_FROM => 'lib/fish_clock_xs.pm',
   # --- cairo is included by X11::Aosd.
   LIBS         => [qw,,], 
   DEFINE       => '',     # e.g., '-DHAVE_SOMETHING'
   INC          => join(' ', uniq @inc),
   MYEXTLIB     => 'mylib/libfish_clock_xs$(LIB_EXT)',
);

sub MY::postamble {
'
$(MYEXTLIB): mylib/Makefile
	cd mylib && $(MAKE) $(PASSTHRU)
';
}

