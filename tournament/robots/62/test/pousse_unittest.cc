#include "pousse.h"
#include "gtest/gtest.h"
#include <algorithm>

TEST(PousseMove, PousseMoveFromString) {
  const PousseMove move1("T14");
  EXPECT_EQ(TOP, move1.direction);
  EXPECT_EQ(14, move1.rank);

  const PousseMove move2("L4");
  EXPECT_EQ(LEFT, move2.direction);
  EXPECT_EQ(4, move2.rank);

  const PousseMove move3("B25");
  EXPECT_EQ(BOTTOM, move3.direction);
  EXPECT_EQ(25, move3.rank);

  const PousseMove move4("R3");
  EXPECT_EQ(RIGHT, move4.direction);
  EXPECT_EQ(3, move4.rank);
}


TEST(PousseMove, PousseMoveToString) {
  const PousseMove move1("T11");
  EXPECT_EQ("T11", move1.toString());
  const PousseMove move2("B21");
  EXPECT_EQ("B21", move2.toString());
  const PousseMove move3("L7");
  EXPECT_EQ("L7", move3.toString());
  const PousseMove move4("R6");
  EXPECT_EQ("R6", move4.toString());
}

TEST(PousseBoard, PousseBoardEmptyConstructor) {
  const PousseBoard board(10);
  EXPECT_EQ(true, board.turn);
  EXPECT_EQ(10, board.dimension);
  EXPECT_EQ(100UL, board.boardX.size());
  EXPECT_EQ(100UL, board.boardO.size());
  EXPECT_EQ(std::vector<bool>(100, false), board.boardX);
  EXPECT_EQ(std::vector<bool>(100, false), board.boardO);
}


TEST(PousseBoard, PousseBoardMoves) {
  const PousseBoard board(12);
  const std::unique_ptr<std::vector<PousseMove> > moves = board.moves();

  EXPECT_EQ(48UL, moves->size());
  EXPECT_NE(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("T8")));
  EXPECT_NE(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("T12")));
  EXPECT_NE(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("B1")));
  EXPECT_NE(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("B7")));
  EXPECT_NE(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("L5")));
  EXPECT_NE(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("L3")));

  EXPECT_EQ(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("T13")));
  EXPECT_EQ(moves->end(), std::find(moves->begin(), moves->end(), PousseMove("T0")));
}


TEST(PousseBoard, PousseBoardMakeMove) {
  const PousseBoard board(4);
  const PousseMove move1("T3");
  const PousseMove move2("R2");
  const PousseBoard board1 = board.makeMove(move1);
  const PousseBoard board2 = board1.makeMove(move1);
  const PousseBoard board3 = board2.makeMove(move1);
  const PousseBoard board4 = board3.makeMove(move1);
  const PousseBoard board5 = board4.makeMove(move1);

  const PousseBoard board6 = board5.makeMove(move1); // should be same as board4

  // fork from board5
  const PousseBoard board7 = board5.makeMove(move2);
  const PousseBoard board8 = board7.makeMove(move2);
  const PousseBoard board9 = board8.makeMove(move2);
  const PousseBoard board10 = board9.makeMove(move2);
  const PousseBoard board11 = board10.makeMove(move2);
  const PousseBoard board12 = board11.makeMove(move2);

  std::vector<bool> teamX(16, false);
  std::vector<bool> teamO(16, false);

  EXPECT_EQ(teamX, board.boardX);
  EXPECT_EQ(teamO, board.boardO);
  EXPECT_EQ(true, board.turn);

  teamX[2] = true;

  EXPECT_EQ(teamX, board1.boardX);
  EXPECT_EQ(teamO, board1.boardO);
  EXPECT_EQ(false, board1.turn);

  teamO[2] = true;
  teamX[2] = false;
  teamX[6] = true;

  EXPECT_EQ(teamX, board2.boardX);
  EXPECT_EQ(teamO, board2.boardO);


  teamX[2] = true;
  teamO[2] = false;
  teamX[6] = false;
  teamO[6] = true;
  teamX[10] = true;

  EXPECT_EQ(teamX, board3.boardX);
  EXPECT_EQ(teamO, board3.boardO);

  teamX[2] = false;
  teamO[2] = true;
  teamX[6] = true;
  teamO[6] = false;
  teamX[10] = false;
  teamO[10] = true;
  teamX[14] = true;

  EXPECT_EQ(teamX, board4.boardX);
  EXPECT_EQ(teamO, board4.boardO);
  EXPECT_EQ(teamX, board6.boardX);
  EXPECT_EQ(teamO, board6.boardO);

  teamX[2] = true;
  teamO[2] = false;
  teamX[6] = false;
  teamO[6] = true;
  teamX[10] = true;
  teamO[10] = false;
  teamX[14] = false;
  teamO[14] = true;

  EXPECT_EQ(teamX, board5.boardX);
  EXPECT_EQ(teamO, board5.boardO);

  teamO[7] = true;

  EXPECT_EQ(teamX, board7.boardX);
  EXPECT_EQ(teamO, board7.boardO);

  //board7 now looks like
  //..X.
  //..OO
  //..X.
  //..O.

  teamX[7] = true;
  teamO[7] = false;
  teamO[5] = true;
  teamO[6] = true;
  teamX[6] = false;

  EXPECT_EQ(teamX, board8.boardX);
  EXPECT_EQ(teamO, board8.boardO);

  teamO[4] = true;
  teamX[6] = true;
  teamO[6] = false;
  teamX[7] = false;
  teamO[7] = true;
  
  EXPECT_EQ(teamX, board9.boardX);
  EXPECT_EQ(teamO, board9.boardO);

  //board9 now looks like
  //..X.
  //OOXO
  //..X.
  //..O.

  teamX[5] = true;
  teamO[5] = false;
  teamX[6] = false;
  teamO[6] = true;
  teamX[7] = true;
  teamO[7] = false;

  EXPECT_EQ(teamX, board10.boardX);
  EXPECT_EQ(teamO, board10.boardO);
  EXPECT_EQ(teamX, board12.boardX);
  EXPECT_EQ(teamO, board12.boardO);

  teamX[4] = true;
  teamO[4] = false;
  teamX[5] = false;
  teamO[5] = true;
  teamX[6] = true;
  teamO[6] = false;
  teamX[7] = false;
  teamO[7] = true;

  EXPECT_EQ(teamX, board11.boardX);
  EXPECT_EQ(teamO, board11.boardO);
}
