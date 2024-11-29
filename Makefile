CXX = g++
CXXFLAGS = -pthread -lrt

SRC = wlb.cpp
OUT = run

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(SRC) -o $(OUT) $(CXXFLAGS)

clean:
	rm -f $(OUT)

.PHONY: all clean
