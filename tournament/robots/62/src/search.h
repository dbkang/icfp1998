#ifndef SEARCH_H
#define SEARCH_H

#include "pousse.h"
//#include <unordered_map>

int alphabeta(PousseGame& game, int depth, int alpha, int beta, Player player, std::vector<PousseMove>& pv);
int searchEval(PousseGame& game, int depth, Player player, std::vector<PousseMove>& pv);

#endif

