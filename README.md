# libelas
Modification of LIBELAS stereo matching library

See README.txt for the original README from LIBELAS.

## Building

(See also README.txt for original build instructions.)

    mkdir build
    cd build
    cmake ..
    make

## Running

Usage:

    elas [options] leftImage rightImage

Command-line options:

    build/elas [options] leftImage rightImage

    -h         show this help information
    -r         save raw disparities as a TIFF file
    -s suffix  add suffix to output file names (default '_disp_')

Default is to write scaled, colorized disparities to .png file
using the given suffix.

