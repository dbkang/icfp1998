#include "pousse.h"
#include "eval.h"
#include <climits>
#include <algorithm>

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

// static evaluator from the perspective of specified player
int eval(const PousseGame& game, Player player) {
  GameState gs = game.result();
  if (gs == X_WINS && player == PLAYER_X) return POUSSE_WIN;
  else if (gs == O_WINS && player == PLAYER_X) return POUSSE_LOSS;
  else if (gs == X_WINS && player == PLAYER_O) return POUSSE_LOSS;
  else if (gs == O_WINS && player == PLAYER_O) return POUSSE_WIN;

  int myBoardValue = boardValue(game.board(player), game.dimension);
  int oppBoardValue = boardValue(game.board(opposite(player)), game.dimension);
  return myBoardValue - oppBoardValue + (player == game.turn() ? VALUE_TURN : -VALUE_TURN);
}
