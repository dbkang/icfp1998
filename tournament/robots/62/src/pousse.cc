#include "pousse.h"

PousseMove::PousseMove(std::string move)  {
  direction = TOP;
  rank = 0;
  if (move.length() > 0) {
    char d = move[0];
    std::string r = move.substr(1);

    if (d == 'B') 
      direction = BOTTOM;
    else if (d == 'L')
      direction = LEFT;
    else if (d == 'R')
      direction = RIGHT;

    rank = std::atoi(r.c_str());
  }
}

std::string PousseMove::toString() const {
  std::string result = "";
  if (direction == BOTTOM)
    result += "B";
  else if (direction == LEFT)
    result += "L";
  else if (direction == RIGHT)
    result += "R";
  else
    result += "T";
  result += std::to_string(rank);
  return result;
}

PousseBoard::PousseBoard (int d) {
  dimension = d;
  turn = true;
  boardX.resize(d * d, false);
  boardO.resize(d * d, false);
}


std::unique_ptr<std::vector<PousseMove> > PousseBoard::moves() const {
  std::unique_ptr<std::vector<PousseMove> > result(new std::vector<PousseMove>());
  for (int i = 1; i <= dimension; i++) {
    result->push_back(PousseMove(TOP, i));
    result->push_back(PousseMove(BOTTOM, i));
    result->push_back(PousseMove(LEFT, i));
    result->push_back(PousseMove(RIGHT, i));
  }
  return result;
}

int PousseBoard::rawIndex(int x, int y) const {
  return (y-1) * dimension + (x-1);
}

// note 1-based numbering
SQUARE_STATE PousseBoard::at(int x, int y) const {
  int raw = rawIndex(x, y);
  return boardX[raw] ? OCCUPIED_X :
    boardO[raw] ? OCCUPIED_O :
    EMPTY;
}


int PousseBoard::calcX(int x, int offset, DIRECTION d) const {
  if (d == TOP || d == BOTTOM) return x;
  else if (d == LEFT) return x + offset;
  else return x - offset;
}


int PousseBoard::calcY(int y, int offset, DIRECTION d) const {
  if (d == LEFT || d == RIGHT) return y;
  else if (d == TOP) return y + offset;
  else return y - offset;
}

PousseBoard PousseBoard::makeMove(PousseMove m) const {
  PousseBoard copy(dimension, !turn, boardX, boardO);
  int x, y, count = 0;
  if (m.direction == TOP || m.direction == BOTTOM) {
    x = m.rank;
    if (m.direction == TOP) y = 1;
    else y = dimension;
  }
  else {
    y = m.rank;
    if (m.direction == LEFT) x = 1;
    else x = dimension;
  }
  while (x >= 1 && x <= dimension && y >= 1 && y <= dimension && at(x, y) != EMPTY) {
    x = calcX(x, 1, m.direction);
    y = calcY(x, 1, m.direction);
    count++;
  }

  if (count == dimension) {
    count--;
    x = calcX(x, -1, m.direction);
    y = calcY(x, -1, m.direction);
  }

  // the little one said roll over
  int raw;
  while (count--) {
    int rawOld = rawIndex(x, y);
    x = calcX(x, -1, m.direction);
    y = calcX(y, -1, m.direction);
    raw = rawIndex(x, y);
    
    copy.boardX[rawOld] = copy.boardX[raw];
    copy.boardO[rawOld] = copy.boardO[raw];
  }
  raw = rawIndex(x, y);

  if (turn) copy.boardX[raw] = true; else copy.boardO[raw] = true;
  return copy;
}


// TODO: Full game state including history.  End of game conditions.

