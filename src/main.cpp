// Modified by Mark Rose from original of Andreas Geiger.
/*
Copyright 2011. All rights reserved.
Institute of Measurement and Control Systems
Karlsruhe Institute of Technology, Germany

This file is part of libelas.
Authors: Andreas Geiger

libelas is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or any later version.

libelas is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libelas; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA 
*/

// Demo program showing how libelas can be used, try "./elas -h" for help

#include <iostream>
#include <unistd.h>
#include "elas.h"
#include "image.h"

#include "opencv2/opencv.hpp"

int writeRaw = 0;
using namespace std;

string suffix = "_disp";

image<uchar>* loadImage(string f) {
  cv::Mat img = cv::imread(f.c_str(), cv::IMREAD_GRAYSCALE);
  image<uchar> *I = new image<uchar>(img.cols, img.rows);
  memcpy(I->data, img.data, img.cols*img.rows);
  return I;
}

void writeImage(string origName, string extension, cv::Mat result) {
  int extensionPos = origName.rfind('.');
  string newName = origName.substr(0, extensionPos) + "_disp" + extension;
  cv::imwrite(newName.c_str(), result);
}

void writeImage(string origName, cv::Mat D, double dispMax) {
  if (writeRaw) {
    writeImage(origName, ".hdr", D);
  } else {
    // Scale and colorize.
    D *= 255.0 / dispMax;
    cv::Mat result;
    D.convertTo(result, CV_8U);

    cv::Mat colorized;
    cv::applyColorMap(result, colorized, cv::COLORMAP_TURBO);

    writeImage(origName, ".png", colorized);
  }
}

// compute disparities of pgm image input pair file_1, file_2
void process(string file_1, string file_2) {

  cout << "Processing: " << file_1 << ", " << file_2 << endl;

  // load images
  image<uchar> *I1 = loadImage(file_1.c_str());
  image<uchar> *I2 = loadImage(file_2.c_str());

  // check for correct size
  if (I1->width()<=0 || I1->height() <=0 || I2->width()<=0 || I2->height() <=0 ||
      I1->width()!=I2->width() || I1->height()!=I2->height()) {
    cout << "ERROR: Images must be of same size, but" << endl;
    cout << "       I1: " << I1->width() <<  " x " << I1->height() << 
                 ", I2: " << I2->width() <<  " x " << I2->height() << endl;
    delete I1;
    delete I2;
    return;    
  }

  // get image width and height
  int32_t width  = I1->width();
  int32_t height = I1->height();

  // allocate memory for disparity images
  const int32_t dims[3] = {width,height,width}; // bytes per line = width
  float* D1_data = (float*)malloc(width*height*sizeof(float));
  float* D2_data = (float*)malloc(width*height*sizeof(float));

  // process
  Elas::parameters param;
  param.postprocess_only_left = false;
  Elas elas(param);
  elas.process(I1->data,I2->data,D1_data,D2_data,dims);

  cv::Mat D1 = cv::Mat(height, width, CV_32F, D1_data);
  cv::Mat D2 = cv::Mat(height, width, CV_32F, D2_data);

  double d1Max, d2Max;
  cv::minMaxLoc(D1, NULL, &d1Max);
  cv::minMaxLoc(D2, NULL, &d2Max);
  double dispMax = max(d1Max, d2Max);

  writeImage(file_1, D1, dispMax);
  writeImage(file_2, D2, dispMax);

  // free memory
  delete I1;
  delete I2;
  free(D1_data);
  free(D2_data);
}

const char *pgmName;

void usage(void) {
  cerr << "usage: " << pgmName << " [options] leftImage rightImage" << endl;
  cerr << "    -h         show this help information" << endl;
  cerr << "    -r         save raw disparities as a TIFF file" << endl;
  cerr << "    -s suffix  add suffix to output file names (default '_disp_')"
       << endl;
  cerr << "Default is to write scaled, colorized disparities to .png file"
       << endl
       << "using the given suffix." << endl;
}

#define RAW_OPT 'r'
#define SUFFIX_OPT 's'

int main(int argc, char* const* argv) {
  pgmName = argv[0];
  int ch;

  string opts;
  opts += RAW_OPT;
  opts += SUFFIX_OPT;
  opts += ':';
  while ((ch = getopt(argc, argv, opts.c_str())) > 0) {
    switch (ch) {
    case RAW_OPT:
      writeRaw = 1;
      break;
    case SUFFIX_OPT:
      suffix = string(optarg);
      break;
    default:
      usage();
      return 1;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 2) {
    usage();
    return 1;
  }

  process(argv[0], argv[1]);
  cout << "... done!" << endl;
  return 0;
}
