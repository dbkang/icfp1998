#include "pousse.h"
#include <iostream>

Player opposite(Player p) {
  return p == PLAYER_X ? PLAYER_O : PLAYER_X;
}

int calcRawIndex(int dimension, int x, int y) {
  return (y-1) * dimension + (x-1);
}

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
  turn = PLAYER_X;
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
  return calcRawIndex(dimension, x, y);
}

// note 1-based numbering
SquareState PousseBoard::at(int x, int y) const {
  int raw = rawIndex(x, y);
  return boardX[raw] ? OCCUPIED_X :
    boardO[raw] ? OCCUPIED_O :
    EMPTY;
}


int PousseBoard::calcX(int x, int offset, Direction d) const {
  if (d == TOP || d == BOTTOM) return x;
  else if (d == LEFT) return x + offset;
  else return x - offset;
}


int PousseBoard::calcY(int y, int offset, Direction d) const {
  if (d == LEFT || d == RIGHT) return y;
  else if (d == TOP) return y + offset;
  else return y - offset;
}

int PousseBoard::straightCount(std::vector<bool> board) const {
  int count = 0;
  for (int x = 1; x <= dimension; x++) {
    for (int y = 1; y <= dimension; y++) {
      if (!board[rawIndex(x, y)]) break;
      if (y == dimension) count++;
    }
  }
  for (int y = 1; y <= dimension; y++) {
    for (int x = 1; x <= dimension; x++) {
      if (!board[rawIndex(x, y)]) break;
      if (x == dimension) count++;
    }
  }
  return count;
}

int PousseBoard::straightCountX() const {
  return straightCount(boardX);
}

int PousseBoard::straightCountO() const {
  return straightCount(boardO);
}

PousseBoard PousseBoard::makeMove(PousseMove m) const {
  PousseBoard copy(dimension, opposite(turn), boardX, boardO);
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
    y = calcY(y, 1, m.direction);
    count++;
  }

  if (count == dimension) {
    count--;
    x = calcX(x, -1, m.direction);
    y = calcY(y, -1, m.direction);
  }
  // the little one said roll over
  int raw;
  while (count--) {
    int rawOld = rawIndex(x, y);
    x = calcX(x, -1, m.direction);
    y = calcY(y, -1, m.direction);
    raw = rawIndex(x, y);
    copy.boardX[rawOld] = copy.boardX[raw];
    copy.boardO[rawOld] = copy.boardO[raw];
  }
  raw = rawIndex(x, y);

  if (turn == PLAYER_X) {
    copy.boardX[raw] = true;
    copy.boardO[raw] = false;
  }
  else {
    copy.boardO[raw] = true;
    copy.boardX[raw] = false;
  }
    
  return copy;
}


void PousseGame::makeMove(PousseMove move) {
  history.push_back(history.back().makeMove(move));
  moveHistory.push_back(move);
}

void PousseGame::undo() {
  history.pop_back();
  moveHistory.pop_back();
}

GameState PousseGame::result() const {
  for (std::vector<PousseBoard>::const_reverse_iterator it = ++(history.crbegin());
       it != history.crend(); ++it) {
    if (history.back() == *it) {
      if (history.back().turn == PLAYER_X) return X_WINS; else return O_WINS;
    }
  }

  int xCount = history.back().straightCountX();
  int oCount = history.back().straightCountO();

  if (xCount > oCount) return X_WINS;
  else if (oCount > xCount) return O_WINS;
  else return IN_PROGRESS;
}


std::vector<bool> PousseGame::board(Player p) const {
  if (p == PLAYER_X) return history.back().boardX;
  else return history.back().boardO;
}

Player PousseGame::turn() const {
  return history.back().turn;
}

PousseBoard PousseGame::currentBoard() const {
  return history.back();
}

std::unique_ptr<std::vector<PousseMove> > PousseGame::moves() const {
  return history.back().moves();
}

std::string PousseGame::toString() const {
  std::string result = "";
  for (int x = 1; x <= dimension; x++) {
    for (int y = 1; y <= dimension; y++) {
      SquareState sq = history.back().at(x, y);
      if (sq == OCCUPIED_X) result += "X";
      else if (sq == OCCUPIED_O) result += "O";
      else result += ".";
    }
    result += "\n";
  }
  return result;
}

std::string PousseGame::movesToString(std::string separator) const {
  std::string result;
  for (std::vector<PousseMove>::const_iterator it = moveHistory.begin(); it != moveHistory.end(); ++it) {
    if (it != moveHistory.begin()) result += separator;
    result += it->toString();
  }
  return result;
}
