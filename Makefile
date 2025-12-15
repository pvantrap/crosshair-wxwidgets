CXX = g++
WX_CONFIG = wx-config

CXXFLAGS = $(shell $(WX_CONFIG) --cxxflags)
LDFLAGS = $(shell $(WX_CONFIG) --libs std,gl) -lGL

TARGET = crosshair_app
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
