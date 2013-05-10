#include "pousse.h"
#include "gtest/gtest.h"
#include <algorithm>

TEST(PousseMove, PousseMoveFromString) {
  const PousseMove move1("T14");
  EXPECT_EQ(move1.direction, TOP);
  EXPECT_EQ(move1.rank, 14);

  const PousseMove move2("L4");
  EXPECT_EQ(move2.direction, LEFT);
  EXPECT_EQ(move2.rank, 4);

  const PousseMove move3("B25");
  EXPECT_EQ(move3.direction, BOTTOM);
  EXPECT_EQ(move3.rank, 25);

  const PousseMove move4("R3");
  EXPECT_EQ(move4.direction, RIGHT);
  EXPECT_EQ(move4.rank, 3);
}


TEST(PousseMove, PousseMoveToString) {
  const PousseMove move1("T11");
  EXPECT_EQ(move1.toString(), "T11");
  const PousseMove move2("B21");
  EXPECT_EQ(move2.toString(), "B21");
  const PousseMove move3("L7");
  EXPECT_EQ(move3.toString(), "L7");
  const PousseMove move4("R6");
  EXPECT_EQ(move4.toString(), "R6");
}

TEST(PousseBoard, PousseBoardEmptyConstructor) {
  const PousseBoard board(10);
  EXPECT_EQ(board.turn, true);
  EXPECT_EQ(board.dimension, 10);
  EXPECT_EQ(board.boardX.size(), 100UL);
  EXPECT_EQ(board.boardO.size(), 100UL);
  EXPECT_EQ(board.boardX, std::vector<bool>(100, false));
  EXPECT_EQ(board.boardO, std::vector<bool>(100, false));
}


TEST(PousseBoard, PousseBoardMoves) {
  const PousseBoard board(12);
  const std::unique_ptr<std::vector<PousseMove> > moves = board.moves();

  EXPECT_EQ(moves->size(), 48UL);
  EXPECT_NE(std::find(moves->begin(), moves->end(), PousseMove("T8")), moves->end());
  EXPECT_NE(std::find(moves->begin(), moves->end(), PousseMove("T12")), moves->end());
  EXPECT_NE(std::find(moves->begin(), moves->end(), PousseMove("B1")), moves->end());
  EXPECT_NE(std::find(moves->begin(), moves->end(), PousseMove("B7")), moves->end());
  EXPECT_NE(std::find(moves->begin(), moves->end(), PousseMove("L5")), moves->end());
  EXPECT_NE(std::find(moves->begin(), moves->end(), PousseMove("L3")), moves->end());

  EXPECT_EQ(std::find(moves->begin(), moves->end(), PousseMove("T13")), moves->end());
  EXPECT_EQ(std::find(moves->begin(), moves->end(), PousseMove("T0")), moves->end());
}


TEST(PousseBoard, PousseBoardMakeMove) {

}
