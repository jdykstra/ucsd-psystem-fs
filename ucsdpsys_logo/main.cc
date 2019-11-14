//
// UCSD p-System filesystem in user space
// Copyright (C) 2006, 2007, 2010 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// you option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>
//

#include <lib/config.h>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <libexplain/fflush.h>
#include <libexplain/output.h>
#include <libexplain/program_name.h>


#define LINE_COLOUR "0.68 0.16 0.00"


static double sx, sy;


static void
coord(double x, double y)
{
    printf("%g %g ", x * sx, y * sy);
}


static void
hexagon(double cx, double cy, const char *colour)
{
    printf("%s setrgbcolor\n", colour);
    coord(cx - 1, cy - 0.5); printf("moveto\n");
    coord(cx,     cy - 1.0); printf("lineto\n");
    coord(cx + 1, cy - 0.5); printf("lineto\n");
    coord(cx + 1, cy + 0.5); printf("lineto\n");
    coord(cx,     cy + 1.0); printf("lineto\n");
    coord(cx - 1, cy + 0.5); printf("lineto\n");
    printf("closepath\n");
    printf("fill\n");

    printf("%s setrgbcolor\n", LINE_COLOUR);
    coord(cx - 1, cy - 0.5); printf("moveto\n");
    coord(cx,     cy - 1.0); printf("lineto\n");
    coord(cx + 1, cy - 0.5); printf("lineto\n");
    coord(cx + 1, cy + 0.5); printf("lineto\n");
    coord(cx,     cy + 1.0); printf("lineto\n");
    coord(cx - 1, cy + 0.5); printf("lineto\n");
    printf("closepath\n");
    printf("stroke\n");
}


static void
stripe1(double x, double y, const char *colour)
{
    printf("%s setrgbcolor\n", colour);
    coord(x,     y      ); printf("moveto\n");
    coord(x + 1, y - 0.5); printf("lineto\n");
    coord(x - 6, y - 4.0); printf("lineto\n");
    coord(x - 6, y - 3.0); printf("lineto\n");
    printf("closepath\n");
    printf("fill\n");

    printf("%s setrgbcolor\n", LINE_COLOUR);
    coord(x,     y      ); printf("moveto\n");
    coord(x + 1, y - 0.5); printf("lineto\n");
    coord(x - 6, y - 4.0); printf("lineto\n");
    coord(x - 6, y - 3.0); printf("lineto\n");
    printf("closepath\n");
    printf("stroke\n");
}


static void
stripe2(double x, double y, const char *colour)
{
    printf("%s setrgbcolor\n", colour);
    coord(x,     y      ); printf("moveto\n");
    coord(x,     y - 1.0); printf("lineto\n");
    coord(x - 6, y - 4.0); printf("lineto\n");
    coord(x - 6, y - 3.0); printf("lineto\n");
    printf("closepath\n");
    printf("fill\n");

    printf("%s setrgbcolor\n", LINE_COLOUR);
    coord(x,     y      ); printf("moveto\n");
    coord(x,     y - 1.0); printf("lineto\n");
    coord(x - 6, y - 4.0); printf("lineto\n");
    coord(x - 6, y - 3.0); printf("lineto\n");
    printf("closepath\n");
    printf("stroke\n");
}


int
main(int argc, char **argv)
{
    explain_program_name_set(argv[0]);
    explain_option_hanging_indent_set(4);
    sx = 56.5;
    if (argc > 1)
    {
        sx = atof(argv[1]);
        if (sx < 20)
            sx = 20;
    }
    sy = sx * (2./sqrt(3.));

    printf("%%!\n");
    printf("gsave\n");
    printf("%g setlinewidth\n", sx/24.);
    hexagon(3.0, 3.5, "0.99 0.57 0.06");
    hexagon(5.0, 3.5, "1.00 0.45 0.10");
    hexagon(7.0, 3.5, "0.76 0.26 0.45");
    hexagon(2.0, 5.0, "0.99 0.71 0.03");
    hexagon(4.0, 5.0, "0.98 0.34 0.11");
    hexagon(6.0, 5.0, "0.89 0.28 0.40");
    hexagon(8.0, 5.0, "0.56 0.23 0.45");
    hexagon(3.0, 6.5, "1.00 0.31 0.13");
    hexagon(5.0, 6.5, "0.95 0.26 0.23");
    hexagon(7.0, 6.5, "0.47 0.26 0.48");

    stripe1(6.0, 3.0, "0.56 0.23 0.44");
    stripe1(4.0, 3.0, "0.98 0.33 0.12");
    stripe1(2.0, 3.0, "0.98 0.44 0.12");
    stripe1(1.0, 4.5, "0.99 0.60 0.04");

    stripe2(2.0, 4.0, "0.98 0.44 0.12");
    stripe2(1.0, 5.5, "0.99 0.60 0.04");
    stripe2(2.0, 7.0, "0.95 0.26 0.23");

    printf("grestore\n");
    printf("showpage\n");

    explain_fflush_or_die(stdout);

    //
    // Once you have the PostScript this program generates,
    // run it through gs, via:
    //
    //    ucsdpsys_logo |
    //    gs -q -sDEVICE=tiff24nc -sOutputFile=logo.tif
    //
    // The using the gimp, open the tif file, and
    //    1. autocrop image
    //    2. change canvas size to 512x512 and slide te image to the
    //       bottom left corner
    //    3. flatten image
    //    4. add transparency layer
    //    5. select by color, no feathering, click on white.
    //    6. save as logo.png
    //
    // If you want text in the lower right corner, select
    // the "URW Bookman L, Bold Italic", size 110
    //
    return 0;
}
