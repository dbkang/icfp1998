#include "eval.h"
#include "search.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <utility>

std::unordered_map<std::string, int> searchResult;
int count;

class MoveCompare {
public:
  PousseGame game;
  std::string moveHistory;
  bool simpleComp;
  // return true if move1 is better than move2
  bool compareSimple(const PousseMove& move1, const PousseMove& move2) {
    // center moves first!
    int dist1 = std::min(game.dimension - move1.rank, move1.rank - 1);
    int dist2 = std::min(game.dimension - move2.rank, move2.rank - 1);
    if (dist1 > dist2) return true;
    else if (dist2 > dist1) return false;
    else return move1.direction <= move2.direction;
  }

  bool operator()(const PousseMove& move1, const PousseMove& move2) {
    if (simpleComp) return compareSimple(move1, move2);
    std::unordered_map<std::string, int>::const_iterator m1 = searchResult.find(moveHistory + move1.toString());
    std::unordered_map<std::string, int>::const_iterator m2 = searchResult.find(moveHistory + move2.toString());
    if (m1 == searchResult.end() || m2 == searchResult.end()) {
      return compareSimple(move1, move2);
    }
    else {
      return m1->second >= m2->second;
    }
      
  }
  MoveCompare(const PousseGame& g, bool s) : game(g), moveHistory(g.movesToString()), simpleComp(s) {}
};


int alphabeta(PousseGame& game, int depth, int alpha, int beta, Player player, std::vector<PousseMove>& pv) {
  PousseMove bestMove("T1");
  if (depth == 0 || game.result() != IN_PROGRESS) {
    count = count + 1;
    return eval(game, player);
  }
  else {
    MoveCompare compare(game, false);
    std::unique_ptr<std::vector<PousseMove> > moves = game.moves();
    if (depth > 1) std::sort(moves->begin(), moves->end(), compare);
    for (std::vector<PousseMove>::iterator it = moves->begin(); it != moves->end(); ++it) {
      std::vector<PousseMove> potentialPV;
      game.makeMove(*it);
      int value = -alphabeta(game, depth - 1, -beta, -alpha, opposite(player), potentialPV);
      if (depth > 1) {
        searchResult[game.movesToString()] = value;
      }
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
  int value;
  for (int i = 1; i <= depth; i++) {
    value = alphabeta(game, depth, POUSSE_LOSS, POUSSE_WIN, player, pv);
    std::reverse(pv.begin(), pv.end());
  }
  std::cerr << count << std::endl;
  return value;
}

// TODO
// sane move ordering
// hashtable
// iterative deepening
