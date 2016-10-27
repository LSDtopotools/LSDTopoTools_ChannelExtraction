//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Lengthscale_correlations.cpp
// David Milodowski September 2014
// Creates spectrally filtered rasters at different lengthscales and compares the
// correlation of the two datasets.
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include "../LSDStatsTools.hpp"
#include "../LSDRaster.hpp"
#include "../LSDRasterSpectral.hpp"
#include "../LSDIndexRaster.hpp"
#include "../LSDFlowInfo.hpp"
#include "../LSDJunctionNetwork.hpp"
#include "../TNT/tnt.h"
int main (int nNumberofArgs,char *argv[])
{
  // ADD ALL PATH RELATED STUFF HERE!
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

  string raster_name;
  string data_path;
  string output_id;
  string flt_ext = "flt";
  string ENVI_ext = "bil";
  string asc_ext = "asc";
  string raster_format;
  string raster_ext;

  string temp;

  file_info_in.open(full_name.c_str());
  if( file_info_in.fail() )
  {
    cout << "\nFATAL ERROR: the header file \"" << full_name
         << "\" doesn't exist" << endl;
    exit(EXIT_FAILURE);
  }
  file_info_in >> temp >> data_path;
  data_path = RemoveControlCharactersFromEndOfString(data_path);

  file_info_in >> temp >> raster_name;
  raster_name = RemoveControlCharactersFromEndOfString(raster_name);

  file_info_in >> temp >> output_id;
  output_id = RemoveControlCharactersFromEndOfString(output_id);

  file_info_in.close();

  // print the spectral data to the data folder.
  output_id = data_path+output_id;


  // now check the raster format
  string lower = raster_format;
  for (unsigned int i=0; i<raster_format.length(); ++i)
  {
    lower[i] = tolower(raster_format[i]);
  }

  if (lower == "bil" || lower == "envi")
  {
    raster_ext = ENVI_ext;
    cout << "You have selected the ENVI bil format" << endl;
  }
  else if (lower == "flt" || lower == "float")
  {
    raster_ext = flt_ext;
    cout << "You have selected the flt format. Note that no georeferencing information" << endl
         << "will be preserved. Perhaps you should try the ENVI bil format?" << endl;
  }
  else if (lower == "asc" || lower == "ascii")
  {
    raster_ext = asc_ext;
    cout << "You have selected the ascii format. Note that no georeferencing information" << endl
         << "will be preserved. Also you are dealing with very large files," << endl
         << "Perhaps you should try the ENVI bil format?" << endl;
  }
  else
  {
    cout << "You did not choose a raster extension, defaulting to ENVI bil format" << endl;
    raster_ext = ENVI_ext;
  }

  // Load in data
  string DEM_f_name = data_path+raster_name;
  LSDRaster raw_raster(DEM_f_name, raster_ext);

  // convert to float by using the polyfit function
  //float window_radius = 61.0;
  //vector<int> raster_selection(8,0);
  //raster_selection[0] = 1;                    // set to return the smoothed surface
  //vector<LSDRaster> polyfit_rasters =
  //    raw_raster.calculate_polyfit_surface_metrics(window_radius, raster_selection);

  // remove the seas
  float sea_threshold = 0.0001;
  raw_raster.mask_to_nodata_below_threshold(sea_threshold);

  // now trim the raster
  LSDRaster trimmed = raw_raster.RasterTrimmerSpiral();

  cout << "Trimmed raster, printing" << endl;

  string Trim_name = "_TRIM";
  trimmed.write_raster((DEM_f_name+Trim_name),raster_ext);

  cout << "Now performing wiener filter" << endl;

  // Perform wiener filter
  LSDRasterSpectral SpectralRaster(trimmed);
  LSDRaster filtered = SpectralRaster.fftw2D_wiener();
  string Filt_name = "_WFILT";
  filtered.write_raster((DEM_f_name+Filt_name),raster_ext);


  // now for a low pass filter (get rid of high frequency noise)
  int FilterType = 2;
  float FLow = 0.02;
  float FHigh = 0.1;
  LSDRaster filtered2 = SpectralRaster.fftw2D_filter(FilterType, FLow, FHigh);
  string Filt_name2 = "_LPFILT";
  filtered2.write_raster((DEM_f_name+Filt_name2),raster_ext);


  // Now get the hillshades
  string hs_name = "_HS";
  LSDRaster hs1 = trimmed.hillshade();
  hs1.write_raster((DEM_f_name+Trim_name+hs_name),raster_ext);
  LSDRaster hs2 = filtered.hillshade();
  hs2.write_raster((DEM_f_name+Filt_name+hs_name),raster_ext);
  LSDRaster hs3 = filtered2.hillshade();
  hs3.write_raster((DEM_f_name+Filt_name2+hs_name),raster_ext);


}