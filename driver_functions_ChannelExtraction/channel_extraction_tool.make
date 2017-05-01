# make with make -f channel_extraction_tool.make

CC=g++
CFLAGS=-c -Wall -O3
OFLAGS = -Wall -O3
LDFLAGS= -Wall
SOURCES=channel_extraction_tool.cpp \
         ../LSDIndexRaster.cpp \
         ../LSDRaster.cpp \
         ../LSDFlowInfo.cpp \
         ../LSDIndexChannel.cpp \
         ../LSDStatsTools.cpp \
         ../LSDRasterSpectral.cpp \
         ../LSDJunctionNetwork.cpp \
         ../LSDChannel.cpp \
         ../LSDRasterInfo.cpp \
         ../LSDParameterParser.cpp \
         ../LSDSpatialCSVReader.cpp \
         ../LSDBasin.cpp \
         ../LSDCRNParameters.cpp \
         ../LSDCosmoData.cpp \
         ../LSDMostLikelyPartitionsFinder.cpp \
         ../LSDShapeTools.cpp
LIBS   = -lm -lstdc++ -lfftw3
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=channel_extraction_tool.exe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
