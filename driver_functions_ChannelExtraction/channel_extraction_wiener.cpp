//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// channel_extraction_perona_malik.cpp
// A driver function for use with the Land Surace Dynamics Topo Toolbox
// This program calculates channel heads using an Optimal wiener filter (used by Pelletier,
// 2013) and a quantile-quantile curvature threshold similar to Geonet.  It then uses a 
// connected components threshold to create a channel network from the curvature mask.
//
// References: Pelletier, J.D. (2013) A robust, two-parameter method for the extraction of
// drainage networks from high-resolution digital elevation models (DEMs): Evaluation using
// synthetic and real-world DEMs, Water Resources Research 49(1): 75-89, doi:10.1029/2012WR012452
//
// Passalacqua, P., Do Trung, T., Foufoula-Georgiou, E., Sapiro, G., & Dietrich, W. E.
// (2010). A geometric framework for channel network extraction from lidar: Nonlinear diffusion and
// geodesic paths. Journal of Geophysical Research: Earth Surface (2003–2012), 115(F1).
//
// He, L., Chao, Y., & Suzuki, K. (2008). A run-based two-scan labeling algorithm. Image Processing,
// IEEE Transactions on, 17(5), 749-756.
//
// Developed by:
//  David T. Milodowski
//  Simon M. Mudd
//  Stuart W.D. Grieve
//  Fiona J. Clubb
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
// Simon M. Mudd, University of Edinburgh
// David T. Milodowski, University of Edinburgh
// Stuart W.D. Grieve, University of Edinburgh
// Fiona J. Clubb, University of Edinburgh
//
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>
#include "../LSDStatsTools.hpp"
#include "../LSDRaster.hpp"
#include "../LSDRasterSpectral.hpp"
#include "../LSDIndexRaster.hpp"
#include "../TNT/tnt.h"
#include "../LSDFlowInfo.hpp"
#include "../LSDJunctionNetwork.hpp"
int main (int nNumberofArgs,char *argv[])
{
  //Test for correct input arguments
  if (nNumberofArgs!=3)
  {
    cout << "FATAL ERROR: wrong number inputs. The program needs the path name, the driver file name" << endl;
    exit(EXIT_SUCCESS);
  }

  string path_name = argv[1];
  string f_name = argv[2];

  cout << "The path is: " << path_name << " and the filename is: " << f_name << endl;

  string full_name = path_name+f_name;

  ifstream file_info_in;
  file_info_in.open(full_name.c_str());
  if( file_info_in.fail() )
  {
    cout << "\nFATAL ERROR: the header file \"" << full_name
         << "\" doesn't exist" << endl;
    exit(EXIT_FAILURE);
  }

  string Raster_name;
  string Output_name;
  string q_q_filename_prefix;
  float area_threshold,window_radius;
  string DEM_extension = "bil";
  string temp;
  int connected_components_threshold;
  file_info_in >> temp >> Raster_name
               >> temp >> Output_name
               >> temp >> q_q_filename_prefix 
               >> temp >> window_radius
               >> temp >> area_threshold
	       >> temp >> connected_components_threshold;
  file_info_in.close();
  // Now create the raster selection vector based on user's selection
  // Elevation
  LSDRasterSpectral raster(Raster_name, DEM_extension);
  LSDIndexRaster connected_components = raster.IsolateChannelsWienerQQ(area_threshold, window_radius, q_q_filename_prefix+".txt");
  cout << "filter by connected components" << endl;
  //LSDIndexRaster output_raster(Output_name,DEM_extension);
  LSDIndexRaster connected_components_filtered = connected_components.filter_by_connected_components(connected_components_threshold);
  LSDIndexRaster CC_raster = connected_components_filtered.ConnectedComponents();
  //LSDIndexRaster output_raster(Output_name,DEM_extension);
  cout << "thin network to skeleton" << endl;
  LSDIndexRaster skeleton_raster = connected_components_filtered.thin_to_skeleton();
  cout << "finding end points" << endl;
  LSDIndexRaster Ends = skeleton_raster.find_end_points();
  Ends.remove_downstream_endpoints(CC_raster, raster);

  //write some rasters
  //connected_components_filtered.write_raster(Output_name+"_cc", DEM_extension);
  //skeleton_raster.write_raster(Output_name+"_skeleton",DEM_extension);
  //Ends.write_raster(Output_name+"_end_points",DEM_extension);
  
  //Now we can process the end points to get only the channel heads - SWDG
  
  cout << "Starting channel head processing" << endl;
  
  //First we need to load the elevation data, fill it and generate a FlowInfo object
  LSDRaster DEM(Raster_name, DEM_extension);
  float MinSlope = 0.0001;
  LSDRaster FilledDEM = DEM.fill(MinSlope);
  vector<string> BoundaryConditions(4, "No Flux");
  LSDFlowInfo FlowInfo(BoundaryConditions,FilledDEM);
  
  //this processes the end points to only keep the upper extent of the channel network
  cout << "getting channel heads" << endl;
  vector<int> tmpsources = FlowInfo.ProcessEndPointsToChannelHeads(Ends);
      
  // we need a temp junction network to search for single pixel channels
  LSDJunctionNetwork tmpJunctionNetwork(tmpsources, FlowInfo);
  LSDIndexRaster tmpStreamNetwork = tmpJunctionNetwork.StreamOrderArray_to_LSDIndexRaster();
  
  cout << "removing single px channels" << endl;
  vector<int> FinalSources = FlowInfo.RemoveSinglePxChannels(tmpStreamNetwork, tmpsources);
  
  //Now we have the final channel heads, so we can generate a channel network from them
  LSDJunctionNetwork JunctionNetwork(FinalSources, FlowInfo);
  LSDIndexRaster StreamNetwork = JunctionNetwork.StreamOrderArray_to_LSDIndexRaster();
  
  //Finally we write the channel network and channel heads to files so they can be used in other drivers. 
  LSDIndexRaster Heads = FlowInfo.write_NodeIndexVector_to_LSDIndexRaster(FinalSources);
  Heads.write_raster((Output_name+"_CH_wiener"),DEM_extension);
  StreamNetwork.write_raster((Output_name+"_SO_wiener"),DEM_extension);
  FlowInfo.print_vector_of_nodeindices_to_csv_file(FinalSources,(Output_name+"_CH_wiener"));
  
  cout << "DONE" << endl;
}
