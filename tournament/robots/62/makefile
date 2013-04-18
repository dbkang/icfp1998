SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
CXX := clang++
GTEST_DIR = gtest-1.6.0
TEST_DIR = test
CPPFLAGS = -I$(GTEST_DIR)/include -I$(SRC_DIR)
CXXFLAGS = -std=c++11 -g -Wall -Wextra
TESTS = pousse_unittest

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

all: $(BIN_DIR)/main test
	./pousse_unittest

$(BIN_DIR)/main : $(OBJ_DIR)/pousse.o $(OBJ_DIR)/main.o $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/main $(OBJ_DIR)/main.o $(OBJ_DIR)/pousse.o

$(OBJ_DIR)/main.o : $(SRC_DIR)/main.cc $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/main.cc

$(OBJ_DIR)/pousse.o : $(SRC_DIR)/pousse.cc $(SRC_DIR)/pousse.h $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $(OBJ_DIR)/pousse.o -c $(SRC_DIR)/pousse.cc

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
	rm -f $(TESTS) gtest.a gtest_main.a *.o

test : $(TESTS)

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

pousse_unittest.o : $(TEST_DIR)/pousse_unittest.cc \
                     $(SRC_DIR)/pousse.h #$(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/pousse_unittest.cc

pousse_unittest : $(OBJ_DIR)/pousse.o pousse_unittest.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ -lpthread


# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

