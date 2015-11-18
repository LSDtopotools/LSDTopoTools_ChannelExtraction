# make with make -f channel_heads.make

CC=g++
CFLAGS=-c -Wall -O3 -pg
OFLAGS = -Wall -O3
LDFLAGS= -Wall
SOURCES=channel_extraction_area_threshold.cpp \
    ../LSDMostLikelyPartitionsFinder.cpp \
    ../LSDIndexRaster.cpp \
    ../LSDRaster.cpp \
    ../LSDFlowInfo.cpp \
    ../LSDJunctionNetwork.cpp \
    ../LSDIndexChannel.cpp \
    ../LSDChannel.cpp \
    ../LSDIndexChannelTree.cpp \
    ../LSDStatsTools.cpp \
    ../LSDShapeTools.cpp
LIBS= -lm -lstdc++ -lfftw3
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=channel_extraction_area_threshold.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
