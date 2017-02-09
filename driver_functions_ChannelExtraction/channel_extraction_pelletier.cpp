//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// channel_extraction_pelletier.cpp
// A driver function for use with the Land Surace Dynamics Topo Toolbox
// This program calculates channel heads using Pelletier (2013)
//
// Reference: Pelletier, J.D. (2013) A robust, two-parameter method for the extraction of
// drainage networks from high-resolution digital elevation models (DEMs): Evaluation using
// synthetic and real-world DEMs, Water Resources Research 49(1): 75-89, doi:10.1029/2012WR012452
//
// Developed by:
//  Fiona Clubb
//  Simon M. Mudd
//  David T. Milodowski
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
// Fiona J. Clubb, University of Edinburgh
// Simon M. Mudd, University of Edinburgh
// David T. Milodowski, University of Edinburgh
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
#include "../LSDRasterSpectral.hpp"
#include "../LSDIndexRaster.hpp"
#include "../LSDFlowInfo.hpp"
#include "../LSDJunctionNetwork.hpp"
#include "../LSDIndexChannelTree.hpp"
#include "../LSDChiNetwork.hpp"

int main (int nNumberofArgs,char *argv[])
{
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

	ifstream file_info_in;
	file_info_in.open(full_name.c_str());
	if( file_info_in.fail() )
	{
		cout << "\nFATAL ERROR: the header file \"" << full_name
		     << "\" doesn't exist" << endl;
		exit(EXIT_FAILURE);
	}

	string DEM_name;
	string fill_ext = "_fill";
	file_info_in >> DEM_name;
	int threshold;
	float Minimum_Slope;
	float curv_threshold;
	float minimum_catchment_area;

	file_info_in >> Minimum_Slope >> threshold >> curv_threshold >> minimum_catchment_area;

	// get some file names
	string DEM_f_name = path_name+DEM_name+fill_ext;
	string DEM_flt_extension = "bil";

	// load the DEM
	LSDRaster topo_test((path_name+DEM_name), DEM_flt_extension);
  LSDRasterSpectral SpectralRaster(topo_test);                        
//   int FilterType = 2;
//   float FLow = 0.01;
//   float FHigh = 0.1;
//   LSDRaster topo_test_filtered = SpectralRaster.fftw2D_filter(FilterType, FLow, FHigh);
  LSDRaster topo_test_wiener = SpectralRaster.fftw2D_wiener();
  int border_width = 100;	
//   topo_test_filtered = topo_test_filtered.border_with_nodata(border_width);
	topo_test_wiener = topo_test_wiener.border_with_nodata(border_width);       
	
	// Set the no flux boundary conditions
  vector<string> boundary_conditions(4);
	boundary_conditions[0] = "No";
	boundary_conditions[1] = "no flux";
	boundary_conditions[2] = "no flux";
	boundary_conditions[3] = "No flux";
	
		// get the filled file
	cout << "Filling the DEM" << endl;
// 	LSDRaster filled_topo_test = topo_test_filtered.fill(Minimum_Slope);
	LSDRaster filled_topo_test = topo_test_wiener.fill(Minimum_Slope);
	filled_topo_test.write_raster((DEM_f_name),DEM_flt_extension);
  
  //get a FlowInfo object
	LSDFlowInfo FlowInfo(boundary_conditions,filled_topo_test); 
	LSDIndexRaster ContributingPixels = FlowInfo.write_NContributingNodes_to_LSDIndexRaster();	
	vector<int> sources;
	sources = FlowInfo.get_sources_index_threshold(ContributingPixels, threshold); 

  // now get the junction network
	LSDJunctionNetwork ChanNetwork(sources, FlowInfo);
	
	// Get the valleys using the contour curvature
	
  int surface_fitting_window_radius = 6;
  int surface_fitting_window_radius_LW = 25;
  vector<LSDRaster> surface_fitting, surface_fitting_LW;
  LSDRaster tan_curvature;
  LSDRaster tan_curvature_LW;
  string curv_name = "_tan_curv";
  vector<int> raster_selection(8, 0);
  raster_selection[6] = 1;
  surface_fitting = topo_test_wiener.calculate_polyfit_surface_metrics(surface_fitting_window_radius, raster_selection);
  surface_fitting_LW = topo_test_wiener.calculate_polyfit_surface_metrics(surface_fitting_window_radius_LW, raster_selection);
  
  for(int i = 0; i<int(raster_selection.size()); ++i)
	{
		if(raster_selection[i]==1)
		{
      tan_curvature = surface_fitting[i];
      tan_curvature.write_raster((path_name+DEM_name+curv_name), DEM_flt_extension);
      tan_curvature_LW = surface_fitting[i];
      tan_curvature_LW.write_raster((path_name+DEM_name+curv_name+"_LW"), DEM_flt_extension);
    }
  }

	string CH_name = "_CH_Pelletier_old";
	Array2D<float> topography = filled_topo_test.get_RasterData();
	Array2D<float> curvature = tan_curvature.get_RasterData();
	Array2D<float> curvature_LW = tan_curvature_LW.get_RasterData();
	cout << "\tLocating channel heads..." << endl;
  vector<int> ChannelHeadNodes = ChanNetwork.calculate_pelletier_channel_heads_DTM(FlowInfo, topography, curv_threshold, curvature,curvature_LW);
  
  // Now filter out false positives along channel according to a threshold 
  // catchment area
  cout << "\tFiltering out false positives..." << endl;
  LSDJunctionNetwork ChanNetworkNew(ChannelHeadNodes, FlowInfo);
	vector<int> ChannelHeadNodesFilt;
  int count = 0;
  for(int i = 0; i<int(ChannelHeadNodes.size()); ++i)
	{
    int upstream_junc = ChanNetworkNew.get_Junction_of_Node(ChannelHeadNodes[i], FlowInfo);
    int test_node = ChanNetworkNew.get_penultimate_node_from_stream_link(upstream_junc, FlowInfo);
    float catchment_area = float(FlowInfo.retrieve_contributing_pixels_of_node(test_node)) * FlowInfo.get_DataResolution();
    if (catchment_area >= minimum_catchment_area) ChannelHeadNodesFilt.push_back(ChannelHeadNodes[i]);
    else ++count;
  }
  cout << "\t...removed " << count << " nodes out of " << ChannelHeadNodes.size() << endl;
  FlowInfo.print_vector_of_nodeindices_to_csv_file(ChannelHeadNodesFilt,(path_name+DEM_name+CH_name));
  //LSDIndexRaster Channel_heads_raster = FlowInfo.write_NodeIndexVector_to_LSDIndexRaster(ChannelHeadNodesFilt);
  //Channel_heads_raster.write_raster((path_name+DEM_name+CH_name),DEM_flt_extension);
	
	//create a channel network based on these channel heads
	LSDJunctionNetwork NewChanNetwork(ChannelHeadNodesFilt, FlowInfo);
  LSDIndexRaster SOArrayNew = NewChanNetwork.StreamOrderArray_to_LSDIndexRaster();
	string SO_name_new = "_SO_Pelletier_old";
	
	SOArrayNew.write_raster((path_name+DEM_name+SO_name_new),DEM_flt_extension);	
                              
}
