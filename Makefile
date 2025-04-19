# Project: orderbook_engine

orderbook: bin/orderbook

CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
INCLUDE = -Iinclude
SRC = src
BUILD = build
BIN = bin

OBJS = $(BUILD)/main.o $(BUILD)/engine.o

bin/orderbook: $(OBJS)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(BUILD)/main.o: $(SRC)/main.cpp include/engine.h
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD)/engine.o: $(SRC)/engine.cpp include/engine.h include/config.h
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(BUILD) $(BIN)