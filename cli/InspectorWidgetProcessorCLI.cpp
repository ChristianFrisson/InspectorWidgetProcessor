/**
 * @file InspectorWidgetProcessorCLI.cpp
 * @brief Command-line interface application for extracting templates and detecting text over video files
 * @author Christian Frisson
 */

#include "InspectorWidgetProcessor.h"

using namespace std;
using namespace cv;

/**
     * @function main
     */
int main( int argc, char** argv )
{
  InspectorWidgetProcessor extractor;

  extractor.init(argc,argv);

  return 0;
}
