#include "pousse.h"
#include "eval.h"
#include <climits>
#include <algorithm>

const int POUSSE_LOSS = INT_MIN;
const int POUSSE_WIN = INT_MAX;

const int VALUE_STONE = 100;
const int VALUE_TURN = 50;
const int VALUE_POSITION_PREMIUM_PRIMARY = 5; // whichever coordinate is closer to center
const int VALUE_POSITION_PREMIUM_SECONDARY = 2; // whichever coordinate is further from center

int boardValue(std::vector<bool> board, int dimension) {
  int value = 0;
  for (int x = 1; x <= dimension; x++) {
    for (int y = 1; y <= dimension; y++) {
      if (board[calcRawIndex(dimension, x, y)]) {
        int xDist = std::min(x - 1, dimension - x);
        int yDist = std::min(y - 1, dimension - y);
        int maxDist = std::max(xDist, yDist);
        int minDist = std::min(xDist, yDist);
        value += VALUE_STONE +
          maxDist * VALUE_POSITION_PREMIUM_PRIMARY +
          minDist * VALUE_POSITION_PREMIUM_SECONDARY;
      }
    }
  }
  return value;
}

// static evaluator from the perspective of whoever's turn it is
int eval(const PousseGame& game, bool fromX) {
  GameState gs = game.result();
  if (gs == X_WINS && fromX) return POUSSE_WIN;
  else if (gs == O_WINS && fromX) return POUSSE_LOSS;
  else if (gs == X_WINS && !fromX) return POUSSE_LOSS;
  else if (gs == O_WINS && !fromX) return POUSSE_WIN;

  int myBoardValue = boardValue(game.board(fromX), game.dimension);
  int oppBoardValue = boardValue(game.board(!fromX), game.dimension);
  return myBoardValue - oppBoardValue + (fromX == game.turn() ? VALUE_TURN : -VALUE_TURN);
}
