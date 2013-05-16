#include <iostream>
#include "pousse.h"
#include "search.h"
#include <ctime>
#include <climits>

void printPV(std::vector<PousseMove>& pv) {
  for(std::vector<PousseMove>::iterator it = pv.begin(); it != pv.end(); ++it) {
    std::cout << it->toString() << " ";
  }
  std::cout << std::endl;
}


int main() {
  PousseGame game(4);
  std::vector<PousseMove> pv;
  int value = searchEval(game, 1, PLAYER_X, pv);
  std::cout << "Level 1: " << value << std::endl;
  printPV(pv);
  value = searchEval(game, 2, PLAYER_X, pv);
  std::cout << "Level 2: " << value << std::endl;
  printPV(pv);
  value = searchEval(game, 3, PLAYER_X, pv);
  std::cout << "Level 3: " << value << std::endl;
  printPV(pv);
  value = searchEval(game, 4, PLAYER_X, pv);
  std::cout << "Level 4: " << value << std::endl;
  printPV(pv);
  value = searchEval(game, 5, PLAYER_X, pv);
  std::cout << "Level 5: " << value << std::endl;
  printPV(pv);
  value = searchEval(game, 6, PLAYER_X, pv);
  std::cout << "Level 6: " << value << std::endl;
  printPV(pv);
  value = searchEval(game, 7, PLAYER_X, pv);
  std::cout << "Level 7: " << value << std::endl;
  printPV(pv);

}

