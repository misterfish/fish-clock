package fish_clock_xs;

use 5.10.0;

use strict;
use warnings;

our $VERSION = '0.01';

use base 'Exporter';
BEGIN { 
    our @EXPORT = qw,,
}

# Nothing further required. Defs are in .xs file(s).

require XSLoader;
XSLoader::load('fish_clock_xs', $VERSION);

1;

