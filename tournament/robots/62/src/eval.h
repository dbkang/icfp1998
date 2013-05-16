#ifndef EVAL_H
#define EVAL_H

#include "pousse.h"

int boardValue(std::vector<bool> board, int dimension);
int eval(const PousseGame& game, bool fromX);

#endif

