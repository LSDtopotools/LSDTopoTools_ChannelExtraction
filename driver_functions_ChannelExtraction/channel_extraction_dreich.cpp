//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// channel_extraction_dreich.cpp
// A driver function for use with the Land Surace Dynamics Topo Toolbox
// This program calculates channel heads using the Dreich method, Clubb et al. (2014)
//
// Reference: Clubb, F. J., S. M. Mudd, D. T. Milodowski, M. D. Hurst,
// and L. J. Slater (2014), Objective extraction of channel heads from
// high-resolution topographic data, Water Resour. Res., 50, doi: 10.1002/2013WR015167.
//
// Developed by:
//  Fiona Clubb
//  Simon M. Mudd
//  David T. Milodowski
//  Stuart W.D. Grieve
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
// Fiona J. Clubb, Univertsity of Edinburgh
// Simon M. Mudd, University of Edinburgh
// David T. Milodowski, University of Edinburgh
// Stuart W.D. Grieve, University of Edinburgh
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
  float area_threshold,window_radius, A_0, m_over_n;
  string DEM_extension = "bil";
  string temp;
  int connected_components_threshold;
  int NJunctions;
  file_info_in >> temp >> Raster_name
               >> temp >> Output_name
               >> temp >> window_radius
               >> temp >> area_threshold
	             >> temp >> connected_components_threshold
	             >> temp >> A_0
	             >> temp >> m_over_n
	             >> temp >> NJunctions;
  file_info_in.close();
  // Now create the raster selection vector based on user's selection
  // Elevation
  LSDRasterSpectral raster(Raster_name, DEM_extension);
  LSDIndexRaster connected_components = raster.IsolateChannelsWienerQQ(area_threshold, window_radius, Output_name+".txt");
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
  string fill_name = "_fill";
  FilledDEM.write_raster((Output_name+fill_name), DEM_extension);
  
  vector<string> BoundaryConditions(4, "No Flux");
  LSDFlowInfo FlowInfo(BoundaryConditions,FilledDEM);
  
  string HS_name = "_HS";
	LSDRaster HS = FilledDEM.hillshade(45, 315, 1);
	HS.write_raster((Output_name+HS_name),DEM_extension);
  
  //this processes the end points to only keep the upper extent of the channel network
  cout << "getting channel heads" << endl;
  vector<int> tmpsources = FlowInfo.ProcessEndPointsToChannelHeads(Ends);
  cout << "processed all end points" << endl;
      
  // we need a temp junction network to search for single pixel channels
  LSDJunctionNetwork tmpJunctionNetwork(tmpsources, FlowInfo);
  LSDIndexRaster tmpStreamNetwork = tmpJunctionNetwork.StreamOrderArray_to_LSDIndexRaster();
  
  cout << "removing single px channels" << endl;
  vector<int> FinalSources = FlowInfo.RemoveSinglePxChannels(tmpStreamNetwork, tmpsources);
  
  // using these sources as the input to run the DrEICH algorithm  - FJC
   
  //Generate a channel netowrk from the sources
  LSDJunctionNetwork JunctionNetwork(FinalSources, FlowInfo);
	LSDIndexRaster JIArray = JunctionNetwork.JunctionIndexArray_to_LSDIndexRaster();
	//string JN_name = "_JI";
	//JIArray.write_raster(Output_name+JN_name, DEM_extension);
  
  LSDIndexRaster StreamNetwork = JunctionNetwork.StreamOrderArray_to_LSDIndexRaster();
  string SO_name = "_SO_valley";
  StreamNetwork.write_raster(Output_name+SO_name, DEM_extension);
      
  LSDRaster DistanceFromOutlet = FlowInfo.distance_from_outlet();
  
	// Calculate the channel head nodes
  int MinSegLength = 10;  
  vector<int> ChannelHeadNodes_temp = JunctionNetwork.GetChannelHeadsChiMethodFromSources(FinalSources, MinSegLength, A_0, m_over_n,
									                    FlowInfo, DistanceFromOutlet, FilledDEM, NJunctions);
									                    
	LSDIndexRaster Channel_heads_raster_temp = FlowInfo.write_NodeIndexVector_to_LSDIndexRaster(ChannelHeadNodes_temp);
										                    							                    
                                                 
  //write channel heads to a raster
  string CH_name = "_CH_DrEICH";
  Channel_heads_raster_temp.write_raster((Output_name+CH_name),DEM_extension);
  
  //write channel heads to csv file
  FlowInfo.print_vector_of_nodeindices_to_csv_file(ChannelHeadNodes_temp,(Output_name+CH_name));

  //create a channel network based on these channel heads
  LSDJunctionNetwork NewChanNetwork(ChannelHeadNodes_temp, FlowInfo);
  //int n_junctions = NewChanNetwork.get_Number_of_Junctions();
  LSDIndexRaster SOArrayNew = NewChanNetwork.StreamOrderArray_to_LSDIndexRaster();
  string SO_name_new = "_SO_DrEICH";

  SOArrayNew.write_raster((Output_name+SO_name_new),DEM_extension);                                               
}
