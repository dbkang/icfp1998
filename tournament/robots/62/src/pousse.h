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
  std::string toString() const;
  bool operator== (const PousseMove& other) const {
    return direction == other.direction && rank == other.rank;
  }
};

class PousseBoard {
public:
  int dimension;
  bool turn; // true is X's turn; false is O's turn
  std::vector<bool> boardX;
  std::vector<bool> boardO;
  PousseBoard (int d, bool t, std::vector<bool> bX, std::vector<bool> bO):
  dimension(d), turn(t), boardX(bX), boardO(bO) {};
  PousseBoard (int d);
  PousseBoard makeMove(PousseMove m) const;
  std::unique_ptr<std::vector<PousseMove> > moves() const;
  SQUARE_STATE at(int x, int y) const;
  int rawIndex(int x, int y) const;
  int calcX(int x, int offset, DIRECTION d) const;
  int calcY(int y, int offset, DIRECTION d) const;
};

#endif
