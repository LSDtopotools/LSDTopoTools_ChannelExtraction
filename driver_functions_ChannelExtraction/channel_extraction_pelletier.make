# make with make -f channel_extraction_pelletier.make

CC=g++
CFLAGS=-c -Wall -O3 -pg
OFLAGS = -Wall -O3 -pg
LDFLAGS= -Wall
SOURCES=channel_extraction_pelletier.cpp \ 
         ../LSDIndexRaster.cpp \
         ../LSDRaster.cpp \
         ../LSDFlowInfo.cpp \ 
         ../LSDIndexChannel.cpp \
         ../LSDStatsTools.cpp \ 
         ../LSDRasterSpectral.cpp \ 
         ../LSDJunctionNetwork.cpp \ 
         ../LSDChannel.cpp \ 
         ../LSDMostLikelyPartitionsFinder.cpp \ 
         ../LSDShapeTools.cpp
LIBS   = -lm -lstdc++ -lfftw3
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=channel_extraction_pelletier.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
