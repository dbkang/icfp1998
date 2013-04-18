#ifndef POUSSE_H
#define POUSSE_H

#include <vector>
#include <string>
#include <cstdlib>
#include <memory>

enum DIRECTION { TOP, BOTTOM, LEFT, RIGHT };
enum SQUARE_STATE { EMPTY, OCCUPIED_X, OCCUPIED_O };

class PousseMove {
public:
  DIRECTION direction;
  int rank;
  PousseMove(std::string move);
  PousseMove(DIRECTION d, int r): direction(d), rank(r) {};
};

class PousseBoard {
public:
  int dimension;
  bool turn; // true is X's turn; false is O's turn
  std::vector<bool> boardX;
  std::vector<bool> boardO;
  PousseBoard (int d, bool t, std::vector<bool> bX, std::vector<bool> bO):
    dimension(d), turn(t), boardX(bX), boardO(bO) {};
  PousseBoard move(PousseMove m);
  std::unique_ptr<std::vector<PousseMove> > moves();
  SQUARE_STATE at(int x, int y);
  int rawIndex(int x, int y);
  int calcX(int x, int offset, DIRECTION d);
  int calcY(int y, int offset, DIRECTION d);
};

#endif
