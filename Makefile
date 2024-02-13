CXX=g++
CXXFLAGS=

BIN_DIR=bin
SRC_DIR=src

CLIENT_SRC=$(SRC_DIR)/client.cpp
SERVER_SRC=$(SRC_DIR)/server.cpp

CLIENT_BIN=$(BIN_DIR)/client
SERVER_BIN=$(BIN_DIR)/server

.PHONY: all
all: directories $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: directories
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(SRC_DIR)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f $(BIN_DIR)/*
