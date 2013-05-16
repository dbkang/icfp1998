#ifndef EVAL_H
#define EVAL_H

#include "pousse.h"
#include <climits>

const int POUSSE_LOSS = INT_MIN;
const int POUSSE_WIN = INT_MAX;

const int VALUE_STONE = 100;
const int VALUE_TURN = 50;
const int VALUE_POSITION_PREMIUM_PRIMARY = 5; // whichever coordinate is closer to center
const int VALUE_POSITION_PREMIUM_SECONDARY = 2; // whichever coordinate is further from center

int boardValue(std::vector<bool> board, int dimension);
int eval(const PousseGame& game, Player player);

#endif

