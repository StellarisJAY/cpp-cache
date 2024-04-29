SRC_DIR=./src
MAIN=$(SRC_DIR)/main.cpp
SRCS = $(filter-out $(MAIN), $(wildcard $(SRC_DIR)/*.cpp))

BUILD_DIR=./build
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))

TARGET=$(BUILD_DIR)/kvstore

TEST_SRCS=$(wildcard $(TEST_DIR)/*.cpp)
TEST_DIR=./test
TEST_TARGET=$(TEST_DIR)/test

DEBUG?=no

CC=g++
CFLAGS=-I$(SRC_DIR) -O3

ifeq ($(DEBUG),yes)
CFLAGS+=-g -D DEBUG
endif


$(TARGET): $(OBJS) $(MAIN)
	@echo g++ -o $@ $^
	@$(CC) -o $@ $^ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo g++ -c -o $@ $<
	@$(CC) -c -o $@ $< $(CFLAGS)

$(TEST_TARGET): $(TEST_SRCS) $(OBJS)
	@$(CC) -o $@ $^ $(CFLAGS)

.PHONY:clean
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TEST_TARGET)

.PHONY:test
test: $(TEST_TARGET)
	@echo running tests...
	@$(TEST_TARGET)
