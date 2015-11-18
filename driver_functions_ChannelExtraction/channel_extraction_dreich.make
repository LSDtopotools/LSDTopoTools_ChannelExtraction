# make with make -f channel_extraction_dreich.make

CC=g++
CFLAGS=-c -Wall -O3 
OFLAGS = -Wall -O3
LDFLAGS= -Wall
SOURCES=channel_extraction_dreich.cpp ../LSDMostLikelyPartitionsFinder.cpp ../LSDIndexRaster.cpp ../LSDRaster.cpp ../LSDRasterSpectral.cpp ../LSDFlowInfo.cpp ../LSDJunctionNetwork.cpp ../LSDIndexChannel.cpp ../LSDChannel.cpp ../LSDIndexChannelTree.cpp ../LSDStatsTools.cpp ../LSDChiNetwork.cpp ../LSDShapeTools.cpp
LIBS= -lm -lstdc++ -lfftw3
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=channel_extraction_dreich.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
