#include "eval.h"
#include "search.h"
#include <iostream>
#include <algorithm>

int alphabeta(PousseGame& game, int depth, int alpha, int beta, Player player, std::vector<PousseMove>& pv) {
  PousseMove bestMove("T1");
  if (depth == 0 || game.result() != IN_PROGRESS) {
    return eval(game, player);
  }
  else {
    std::unique_ptr<std::vector<PousseMove> > moves = game.moves();
    for (std::vector<PousseMove>::iterator it = moves->begin(); it != moves->end(); ++it) {
      std::vector<PousseMove> potentialPV;
      game.makeMove(*it);
      int value = -alphabeta(game, depth - 1, -beta, -alpha, opposite(player), potentialPV);
      game.undo();
      if (value > alpha) {
        alpha = value;
        bestMove = *it;
        pv = potentialPV;
      }
      if (value >= beta) break;
    }
  }
  pv.push_back(bestMove);
  return alpha;
}

int searchEval(PousseGame& game, int depth, Player player, std::vector<PousseMove>& pv) {
  int value = alphabeta(game, depth, POUSSE_LOSS, POUSSE_WIN, player, pv);
  std::reverse(pv.begin(), pv.end());
  return value;
}

// TODO
// sane move ordering
// hashtable
// iterative deepening
