#include "deals.h"
#include "node.h"

int main(int argc, char* argv[]) {
  int seed = 1;
  if (argc > 1) seed = atoi(argv[1]);

  Node layout;
  layout.set_cards(GetDeal(seed));

  auto moves = DecodeSolution(ReadSolution(seed));
  for (const auto& move : moves) {
    layout.PlayMoves({move});
    layout.Show();
  }
  return 0;
}
