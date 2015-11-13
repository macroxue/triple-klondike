#include "move.h"
#include "node.h"

void Move::Show() const {
  if (value_ == 0) {
  } else if (value_ == 1)
    printf("==> flip stock");
  else if (value_ == 2)
    printf("==> waste to foundation");
  else if (value_ < 16)
    printf("==> waste to tableau %d", value_ - 3);
  else if (value_ < 29)
    printf("==> tableau %d to foundation", value_ - 16);
  else if (value_ < 29 + 169)
    printf("==> tableau %d to tableau %d", (value_ - 29) / 13,
           (value_ - 29) % 13);
  else
    printf("==> foundation to tableau %d", (value_ - 29 - 169));
}

string Move::Encode(Node* play) const {
  assert(value_ != 0);
  if (value_ == 1) {
    play->FlipStock();
    return string("@");
  } else if (value_ == 2) {
    int f = play->FindFoundation(play->stock().Top());
    play->WasteToFoundation(f);
    return string("a") + char('b' + f);
  } else if (value_ < 16) {
    int t = value_ - 3;
    play->WasteToTableau(t);
    return string("a") + char('n' + t);
  } else if (value_ < 29) {
    int t = value_ - 16;
    int f = play->FindFoundation(play->tableau(t).Top());
    play->TableauToFoundation(t, f);
    return string(1, 'n' + t) + char('b' + f);
  } else if (value_ < 29 + 169) {
    int s = (value_ - 29) / 13;
    int t = (value_ - 29) % 13;
    play->TableauToTableau(s, t);
    return string(1, 'n' + s) + char('n' + t);
  } else {
    int t = value_ - 29 - 169;
    int f = play->FindFoundation(play->tableau(t).Top());
    play->FoundationToTableau(f, t);
    return string(1, 'b' + f) + char('n' + t);
  }
}

void PlainMove::Show() const {
  switch (type) {
    case kNone:
      return;
    case kFlipStock:
      return (void)printf("==> flip stock");
    case kWasteToFoundation:
      return (void)printf("==> waste to foundation");
    case kWasteToTableau:
      return (void)printf("==> waste to tableau %d", to);
    case kTableauToFoundation:
      return (void)printf("==> tableau %d to foundation", from);
    case kTableauToTableau:
      return (void)printf("==> tableau %d to tableau %d", from, to);
    case kFoundationToTableau:
      return (void)printf("==> foundation to tableau %d", to);
  }
}

string PlainMove::Encode() const {
  switch (type) {
    case kFlipStock:
      return "@";
    case kWasteToFoundation:
      return string("a") + char('b' + to);
    case kWasteToTableau:
      return string("a") + char('n' + to);
    case kTableauToFoundation:
      return string("") + char('n' + from) + char('b' + to);
    case kTableauToTableau:
      return string("") + char('n' + from) + char('n' + to);
    case kFoundationToTableau:
      return string("") + char('b' + from) + char('n' + to);
    default:
      return "";
  }
}

string ReadSolution(int seed) {
  int solution_seed;
  char solution_code[1024];
  int items = scanf("%d%s", &solution_seed, solution_code);
  if (items != 2) {
    puts("failed to read solution");
    exit(1);
  }
  assert(solution_seed == seed);
  return solution_code;
}

vector<PlainMove> DecodeSolution(const string& code) {
  vector<PlainMove> moves;
  for (int i = 0; i < code.size(); ++i) {
    if (code[i] == '@')
      moves.push_back(PlainMove(code[i]));
    else {
      moves.push_back(PlainMove(code[i], code[i + 1]));
      ++i;
    }
  }
  return moves;
}
