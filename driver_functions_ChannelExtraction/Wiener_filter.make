# make with make -f Wiener_filter.make

CC=g++
CFLAGS=-c -Wall -O3
OFLAGS = -Wall -O3
LDFLAGS= -Wall
SOURCES=Wiener_filter.cpp \
        ../LSDIndexRaster.cpp \
        ../LSDRaster.cpp \
        ../LSDFlowInfo.cpp \
        ../LSDIndexChannel.cpp \
        ../LSDStatsTools.cpp \
        ../LSDRasterSpectral.cpp \
        ../LSDJunctionNetwork.cpp \
        ../LSDMostLikelyPartitionsFinder.cpp \
        ../LSDChannel.cpp \
        ../LSDShapeTools.cpp
LIBS   = -lm -lstdc++ -lfftw3
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Wiener_filter.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
