Rebecca Edson
CPSC 4040 Final Project
Limited Color Palette Mapping
Fall 2019

Program can read image from file, display image, map image to limited
palette and display, map it to palette with Floyd-Steinberg dithering
and display, and write those images to files.

How to run code:
Compile with Makefile ("make").
Run with "./limited <input image filename> [<normal output image filename>]
  [<dithered output image filename>]" without quotes and arguments in
  brackets being optional (if including dithered output image filename, must
  also include one for normal, in same order as above).
When prompted with, "Do you wish to use an existing color palette from a file?
  (Y/N)," press 'Y' (or 'y') if you would like to use a palette from a file in
  this directory, or press 'N' (or 'n'), if you would like to create a new
  palette in the terminal.
If 'Y' - Enter name of palette file (including .txt extension) when prompted.
If 'N' - Enter number of colors in palette when prompted, and enter RGB values
  (separated by space) of each color when prompted (ex. Color 2: 255 255 255).
Press 'n' key to display image where each pixel is mapped to the closest color
  in the palette.
Press 'd' key to display image where the error of each palette match is
  distributed to the neighboring pixels using Floyd-Steinberg dithering.
Press 'r' key to reload original image.
Press ESC to exit program/close window.

Assuming valid commandline arguments - no input validation.
Assuming .png images, .txt palettes, and RGB images with no alpha channel.
Images and filters only supported if within same directory as program (no
  subdirectories).
