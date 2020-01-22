/********************************
  Rebecca Edson
  CPSC 4040 Final Project
  Limited Color Palette Mapping
  Fall 2019
********************************/


#include <OpenImageIO/imageio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cmath>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING


struct rgb_pixel {
  unsigned char r, g, b;
};

rgb_pixel *palette, *original, *limited, *dithered;
int n, width, height;
string out_image_lim;
string out_image_dith;
bool dither_file = 0;
bool limited_file = 0;
bool already_dithered = 0;
bool already_limited = 0;
bool display_dithered = 0;
bool display_limited = 0;
bool display_original = 1;


/*
  Routine to read image file and store in pixmap
*/
void readImage(string file) {

  ImageInput *in = ImageInput::open(file);  // open file
	if(!in) {
    cerr << "Could not open: " << file << ", error = ";
    cerr << geterror() << endl;
    return;
  }

	// get width and height of image
  const ImageSpec &spec = in->spec();
  width = spec.width;
  height = spec.height;

	// pixmap to store image
	original = new rgb_pixel[width*height];

	// read image file, map pixels
  if(!in->read_image(TypeDesc::UINT8, original)) {
    cerr << "Could not read pixels from: " << file << ", error = ";
    cerr << geterror() << endl;
    ImageInput::destroy(in);
    return;
  }

	// close file
  if(!in->close()) {
    cerr << "Error closing: " << file << ", error = ";
    cerr << geterror() << endl;
    ImageInput::destroy(in);
  }

  ImageInput::destroy(in);

}


/*
  Read limited palette file and store in array of RGB pixels
*/
void parsePalette(const char *pal_file) {

  ifstream file(pal_file);  // open palette file
  int r, g, b;

  if(file) {
    file >> n;  // number of colors

    // RGB values for each color
    for(int i = 0; i < n; i++) {
      file >> r >> g >> b;
      palette[i].r = r;
      palette[i].g = g;
      palette[i].b = b;
    }
  }

}


/*
  Set image color to "closest" match in palette
*/
rgb_pixel colorMatch(rgb_pixel image_pixel) {

  rgb_pixel closest;
  double min_distance;
  unsigned char r1, g1, b1, r2, g2, b2;

  // original color
  r1 = image_pixel.r;
  g1 = image_pixel.g;
  b1 = image_pixel.b;

  // find minimum distance between image and palette color
  for(int i = 0; i < n; i++) {
    // valid palette color
    r2 = palette[i].r;
    g2 = palette[i].g;
    b2 = palette[i].b;
    // Euclidian distance
    double distance = sqrt(pow(r2-r1,2)+pow(g2-g1,2)+pow(b2-b1,2));
    if(i == 0 || distance < min_distance) {
      min_distance = distance;  // shortest distance saved
      closest = palette[i];  // palette color with shortest distance to actual color
    }
  }

  return closest;

}


/*
  Create pixmap of each pixel in image mapped to limited palette
*/
void mapImage() {

  limited = new rgb_pixel[width*height];

  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      int k = i * width + j;
      limited[k] = colorMatch(original[k]);
    }
  }

}


/*
  Create pixmap of image mapped to limited palette with Floyd-Steinberg
  dithering technique
*/
void ditherImage() {

  // copy original image to pixmap
  dithered = new rgb_pixel[width*height];
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      int k = i * width + j;
      dithered[k].r = original[k].r;
      dithered[k].g = original[k].g;
      dithered[k].b = original[k].b;
    }
  }

  // set each pixel value
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      int k = i * width + j;
      rgb_pixel old_pixel = dithered[k];  // pixel currently in pixmap
      // set to closest match in palette
      rgb_pixel new_pixel = colorMatch(old_pixel);
      dithered[k] = new_pixel;

      // quantization error = difference between actual and palette color
      // signed
      char r_error = old_pixel.r - new_pixel.r;
      char g_error = old_pixel.g - new_pixel.g;
      char b_error = old_pixel.b - new_pixel.b;

      // indexes of surrounding pixels
      int r = i * width + (j+1);  // right
      int bl = (i+1) * width + (j-1);  // bottom left
      int b = (i+1) * width + j;   // bottom
      int br = (i+1) * width + (j+1);  // bottom right

      // error distribution
      // pixels with sufficient surrounding pixels for algorithm
      if(j+1 < width && j-1 >= 0 && i+1 < height) {
        // pixel to right
        dithered[r].r = dithered[r].r + r_error*7.0/16;
        dithered[r].g = dithered[r].g + g_error*7.0/16;
        dithered[r].b = dithered[r].b + b_error*7.0/16;
        // pixel on bottom left
        dithered[bl].r = dithered[bl].r + r_error*3.0/16;
        dithered[bl].g = dithered[bl].g + g_error*3.0/16;
        dithered[bl].b = dithered[bl].b + b_error*3.0/16;
        // pixel directly below
        dithered[b].r = dithered[b].r + r_error*5.0/16;
        dithered[b].g = dithered[b].g + g_error*5.0/16;
        dithered[b].b = dithered[b].b + b_error*5.0/16;
        // pixel on bottom right
        dithered[br].r = dithered[br].r + r_error*1.0/16;
        dithered[br].g = dithered[br].g + g_error*1.0/16;
        dithered[br].b = dithered[br].b + b_error*1.0/16;
      }
    }
  }

}


/*
  Routine to write limited palette image pixmap to file
*/
void writeImage(string file, rgb_pixel *pixmap) {

	// create oiio file handler for image
  ImageOutput *out = ImageOutput::create(file);
  if(!out) {
    cerr << "Could not create: " << file << ", error = ";
    cerr << OpenImageIO::geterror() << endl;
    return;
  }

  // open file for writing image
  // file header will indicate RGB image with dimensions of warped image
  ImageSpec spec(width, height, 3, TypeDesc::UINT8);
  if(!out->open(file, spec)) {
    cerr << "Could not open " << file << ", error = ";
    cerr << geterror() << endl;
    ImageOutput::destroy(out);
    return;
  }

  // write image to file
  if(!out->write_image(TypeDesc::UINT8, pixmap)) {
    cerr << "Could not close " << file << ", error = ";
    cerr << geterror() << endl;
    ImageOutput::destroy(out);
    return;
  }

  ImageOutput::destroy(out);

}


/*
  Displays image on pixmap
*/
void displayImage() {

  rgb_pixel *pixmap;
  if(display_original)
    pixmap = original;  // display original image
  else if(display_limited)
    pixmap = limited;  // display individually mapped pixel image
  else
    pixmap = dithered;  // display dithered image

  glClearColor(0, 0, 0, 0);  // clear background
  glClear(GL_COLOR_BUFFER_BIT);  // clear window

  glPixelZoom(1, -1);  // flip image vertically
  glRasterPos2i(-1, 1);  // set drawing position to top left corner

  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixmap);  // display pixmap

  glFlush();  // flush OpenGL pipeline to viewport

}


/*
  Keyboard callback routine:
  'r' - reload original
  'n' - normal map
  'd' - dither
  ESC - quit
*/
void handleKey(unsigned char key, int x, int y) {

  switch(key) {
    case 'r':
    case 'R':
      // display original = TRUE
      display_original = 1;
      display_limited = 0;
      display_dithered = 0;
      glutPostRedisplay();  // display
      break;

    case 'n':
    case 'N':
      if(!already_limited) {
        mapImage();  // create pixmap
        already_limited = 1;  // pixmap already created
        // write image to file if filename specified
        if(limited_file)
          writeImage(out_image_lim, limited);
      }
      // display individually mapped pixel image = TRUE
      display_limited = 1;
      display_original = 0;
      display_dithered = 0;
      glutPostRedisplay();  // display
      break;

    case 'd':
    case 'D':
      if(!already_dithered) {
        ditherImage();  // create pixmap
        already_dithered = 1;  // pixmap already created
        // write image to file if filename specified
        if(dither_file)
          writeImage(out_image_dith, dithered);
      }
      // display dithered image = TRUE
      display_dithered = 1;
      display_original = 0;
      display_limited = 0;
      glutPostRedisplay();  // display
      break;

    // ESC
    case 27:
      exit(0);  // quit
      break;

    default:
      return;
  }

}


int main(int argc, char *argv[]) {

  // user chooses preset palette within directory
  cout << "Do you wish to use an existing color palette from a file? (Y/N)" << endl;
  char answer;
  cin >> answer;
  if(answer == 'Y' || answer == 'y') {
    // enter filename including .txt extension
    cout << "Please enter the name of the .txt file: ";
    string pal_file_str;
    cin >> pal_file_str;
    const char *pal_file = pal_file_str.c_str();
    palette = new rgb_pixel[n];
    parsePalette(pal_file);  // get color data from file
  }
  // user chooses to create own palette
  else if(answer == 'N' || answer == 'n') {
    // enter number of colors to appear in palette
    cout << "Number of colors in palette: " << endl;
    cin >> n;
    palette = new rgb_pixel[n];
    int r, g, b;
    // enter each RGB value separated by spaces
    for(int i = 0; i < n; i++) {
      cout << "Color " << i+1 << ": ";
      cin >> r >> g >> b;
      palette[i].r = r;
      palette[i].g = g;
      palette[i].b = b;
    }
  }

  // read input image
  string in_image = argv[1];
  readImage(in_image);

  // optionally specified filenames
  if(argc > 2) {
    out_image_lim = argv[2];  // filename for individually mapped pixel image
    limited_file = 1;  // specified = TRUE
    if(argc > 3) {
      out_image_dith = argv[3];  // filename for dithered image
      dither_file = 1;  // specified = TRUE
    }
  }

  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(width, height);
  glutCreateWindow("limited");

  glutKeyboardFunc(handleKey);
  glutDisplayFunc(displayImage);

  glutMainLoop();

  return 0;

}
