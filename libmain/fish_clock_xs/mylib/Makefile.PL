# Everything in this dir gets compiled and linked into 'myextlib'
# libfish_clock_xs.

use ExtUtils::MakeMaker;

use List::MoreUtils 'uniq';

my @inc = qw,
    -I.
    -I/usr/include/cairo -I/usr/include/glib-2.0
    -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/pixman-1
    -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/libdrm
    
,;

$Verbose = 1;
WriteMakefile(
   NAME   => 'fish_clock_xs::fish_clock_xs', # not so important in compiling 
   SKIP   => [qw(all static static_lib dynamic dynamic_lib)],
   clean  => {'FILES' => 'libfish_clock_xs$(LIB_EXT)'},
   INC          => join(' ', uniq @inc),
);


sub MY::top_targets {
       '
all :: static

pure_all :: static

static ::       libfish_clock_xs$(LIB_EXT)

# real tabs
libfish_clock_xs$(LIB_EXT): $(O_FILES)
	$(AR) cr libfish_clock_xs$(LIB_EXT) $(O_FILES)
	$(RANLIB) libfish_clock_xs$(LIB_EXT)

';
}

