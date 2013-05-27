SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
CXX := clang++
GTEST_DIR = gtest-1.6.0
TEST_DIR = test
CPPFLAGS = -I$(GTEST_DIR)/include -I$(SRC_DIR)
CXXFLAGS = -std=c++11 -stdlib=libc++ -g -Wall -Wextra
TESTS = $(BIN_DIR)/pousse_unittest

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

all: $(BIN_DIR) $(BIN_DIR)/main test
	$(BIN_DIR)/pousse_unittest

$(BIN_DIR)/main : $(OBJ_DIR)/pousse.o $(OBJ_DIR)/main.o $(OBJ_DIR)/eval.o $(OBJ_DIR)/search.o
	$(CXX) $(CXXFLAGS) $^ -o $@ 

$(OBJ_DIR)/main.o : $(SRC_DIR)/main.cc $(SRC_DIR)/pousse.h $(SRC_DIR)/search.h $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $(SRC_DIR)/main.cc

$(OBJ_DIR)/pousse.o : $(SRC_DIR)/pousse.cc $(SRC_DIR)/pousse.h $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $(SRC_DIR)/pousse.cc

$(OBJ_DIR)/eval.o : $(SRC_DIR)/eval.cc $(SRC_DIR)/eval.h $(SRC_DIR)/pousse.h $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $(SRC_DIR)/eval.cc

$(OBJ_DIR)/search.o : $(SRC_DIR)/search.cc $(SRC_DIR)/eval.h $(SRC_DIR)/pousse.h $(SRC_DIR)/search.h $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $(SRC_DIR)/search.cc

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)

test : $(TESTS)

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

$(OBJ_DIR)/pousse_unittest.o : $(TEST_DIR)/pousse_unittest.cc \
                     $(SRC_DIR)/pousse.h $(SRC_DIR)/eval.h #$(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/pousse_unittest.cc -o $@

$(BIN_DIR)/pousse_unittest : $(OBJ_DIR)/pousse.o $(OBJ_DIR)/pousse_unittest.o $(OBJ_DIR)/gtest_main.a $(OBJ_DIR)/eval.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ -lpthread


# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
$(OBJ_DIR)/gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o $@

$(OBJ_DIR)/gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o $@

$(OBJ_DIR)/gtest.a : $(OBJ_DIR)/gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)/gtest_main.a : $(OBJ_DIR)/gtest-all.o $(OBJ_DIR)/gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

