//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// channel_extraction_area_threshold.cpp
// A driver function for use with the Land Surace Dynamics Topo Toolbox
// This program calculates channel heads using a contributing area threshold that should
// be specified by the user in a parameter file.
//
// Developed by:
//  Fiona Clubb
//  Simon M. Mudd
//
// Copyright (C) 2013 Fiona Clubb and Simon M. Mudd 2013
//
// Developer can be contacted by simon.m.mudd _at_ ed.ac.uk
//
//    Simon Mudd
//    University of Edinburgh
//    School of GeoSciences
//    Drummond Street
//    Edinburgh, EH8 9XP
//    Scotland
//    United Kingdom
//
// This program is free software;
// you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY;
// without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the
// GNU General Public License along with this program;
// if not, write to:
// Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301
// USA
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Fiona Clubb, University of Edinburgh
//
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <string.h>
#include "../LSDStatsTools.hpp"
#include "../LSDRaster.hpp"
#include "../LSDIndexRaster.hpp"
#include "../LSDFlowInfo.hpp"
#include "../LSDJunctionNetwork.hpp"
#include "../LSDIndexChannelTree.hpp"
#include "../LSDChiNetwork.hpp"

int main (int nNumberofArgs,char *argv[])
{

  cout << "Welcome to the threshold area channel extraction program. This program finds channel\n"
       << "heads using a contributing area threshold\n"
       << "To run this program you need a driver file with the name of the DEM\n"
       << "WITHOUT extension, the minimum slope, and a threshold number of pixels for\n"
       << "intial channel identification.\n\n"
       << "Here is an example:\n"
       << "indian_creek\n"
       << "0.0001\n"
       << "300\n"
       << endl;

  //Test for correct input arguments
  if (nNumberofArgs!=3)
  {
    cout << "FATAL ERROR: wrong number inputs. The program needs the path name and the file name" << endl;
    exit(EXIT_SUCCESS);
  }

  string path_name = argv[1];
  string f_name = argv[2];

  cout << "The path is: " << path_name << " and the filename is: " << f_name << endl;

  string full_name = path_name+f_name;
  cout << "The full path is: " << full_name << endl;

  ifstream file_info_in;
  file_info_in.open(full_name.c_str());
  if( file_info_in.fail() )
  {
    cout << "\nFATAL ERROR: the driver file \"" << full_name
         << "\" doesn't exist" << endl;
    exit(EXIT_FAILURE);
  }

  string DEM_name;
  string fill_ext = "_fill";
  string sources_ext = "_CH";
  file_info_in >> DEM_name;
  int threshold;
  float Minimum_Slope;

  file_info_in >> Minimum_Slope >> threshold;

  // get some file names
  string DEM_f_name = DEM_name+fill_ext;
  string DEM_flt_extension = "bil";
  string complete_fname = path_name+DEM_name+sources_ext;		//".bil";
  string DEM_with_path = path_name+DEM_name;

  cout << "I am running the channel finding algorithm with the following parameters: " << endl;
  cout << "Minimum slope: " << Minimum_Slope << " threshold: " << threshold << endl;

  // load the DEM
  LSDRaster topo_test(DEM_with_path, DEM_flt_extension);

  // Set the no flux boundary conditions
  vector<string> boundary_conditions(4);
  boundary_conditions[0] = "No";
  boundary_conditions[1] = "no flux";
  boundary_conditions[2] = "no flux";
  boundary_conditions[3] = "No flux";

  // get the filled file
  cout << "Filling the DEM" << endl;
  LSDRaster filled_topo_test = topo_test.fill(Minimum_Slope);
  filled_topo_test.write_raster((path_name+DEM_f_name),DEM_flt_extension);

  //get a FlowInfo object
  LSDFlowInfo FlowInfo(boundary_conditions,filled_topo_test);
  LSDRaster DistanceFromOutlet = FlowInfo.distance_from_outlet();
  LSDIndexRaster ContributingPixels = FlowInfo.write_NContributingNodes_to_LSDIndexRaster();

  //get the sources: note: this is only to select basins!
  vector<int> sources;
  sources = FlowInfo.get_sources_index_threshold(ContributingPixels, threshold);

  // now get the junction network
  LSDJunctionNetwork ChanNetwork(sources, FlowInfo);

  //write channel_heads to a csv file
  FlowInfo.print_vector_of_nodeindices_to_csv_file(sources, complete_fname);

  //write channel heads to a raster
  string CH_name = "_CH";
  LSDIndexRaster Channel_heads_raster = FlowInfo.write_NodeIndexVector_to_LSDIndexRaster(sources);
  Channel_heads_raster.write_raster((path_name+DEM_name+CH_name),DEM_flt_extension);

  //write stream order array to a raster
  LSDIndexRaster SOArray = ChanNetwork.StreamOrderArray_to_LSDIndexRaster();
  string SO_name = "_SO";

  SOArray.write_raster((path_name+DEM_name+SO_name),DEM_flt_extension);

}
