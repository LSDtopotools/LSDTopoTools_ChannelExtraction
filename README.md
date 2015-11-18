# Channel extraction using LSDTopoTools
This repository contains all the code needed for the different methods of channel extraction implemented in LSDTopoTools.
There are four different channel extraction methods which can be found in the folder `driver_functions_ChannelExtraction`:

1. Area threshold - `channel_extraction_area_threshold.cpp` and `channel_extraction_area_threshold.make`
2. Pelletier algorithm (Pelletier, 2013) - `channel_extraction_pelletier.cpp` and `channel_extraction_pelletier.make`
3. DrEICH algorithm (Clubb et al., 2014) - `channel_extraction_dreich.cpp` and `channel_extraction_dreich.make`
4. Geometric algorithm using Wiener filter - `channel_extraction_wiener.cpp` and `channel_extraction_wiener.make`

For instructions and full documentation on how to run each of these methods please see the [LSDTopoTools website](http://lsdtopotools.github.io/LSDTT_book/).  Example drivers for each of the methods can be found in BLAH repository.
