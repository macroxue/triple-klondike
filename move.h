#ifndef MOVE_H
#define MOVE_H

#include <string>
#include <vector>
using namespace std;

#include <assert.h>
#include "card.h"

enum MoveType {
  kNone,
  kFlipStock,
  kWasteToFoundation,
  kWasteToTableau,
  kTableauToFoundation,
  kTableauToTableau,
  kFoundationToTableau
};

class Node;

class Move {
 public:
  Move() : value_(0) {}
  Move(MoveType type) : Move(type, 0, 0) {}
  Move(MoveType type, int to) : Move(type, 0, to) {}
  Move(MoveType type, int from, int to) {
    switch (type) {
      case kNone:
        value_ = 0;
        break;
      case kFlipStock:
        value_ = 1;
        break;
      case kWasteToFoundation:
        value_ = 2;
        break;
      case kWasteToTableau:
        value_ = 3 + to;
        break;
      case kTableauToFoundation:
        value_ = 16 + from;
        break;
      case kTableauToTableau:
        value_ = 29 + from * 13 + to;
        break;
      case kFoundationToTableau:
        value_ = 29 + 169 + to;
        break;
    }
  }

  MoveType type() const {
    if (value_ == 0)
      return kNone;
    else if (value_ == 1)
      return kFlipStock;
    else if (value_ == 2)
      return kWasteToFoundation;
    else if (value_ < 16)
      return kWasteToTableau;
    else if (value_ < 29)
      return kTableauToFoundation;
    else if (value_ < 29 + 169)
      return kTableauToTableau;
    else
      return kFoundationToTableau;
  }

  void Show() const;
  string Encode(Node* play) const;

 private:
  unsigned char value_;
};

struct PlainMove {
  PlainMove() : PlainMove(kNone, 0, 0) {}
  PlainMove(MoveType type, int from, int to) : type(type), from(from), to(to) {}

  PlainMove(char from) : PlainMove(kFlipStock, 0, 0) { assert(from == '@'); }

  PlainMove(char from, char to) {
    assert(from != '@');
    if (from == 'a') {
      if (to < 'n')
        *this = PlainMove(kWasteToFoundation, 0, to - 'b');
      else
        *this = PlainMove(kWasteToTableau, 0, to - 'n');
    } else if (from < 'n') {
      *this = PlainMove(kFoundationToTableau, from - 'b', to - 'n');
    } else {
      if (to < 'n')
        *this = PlainMove(kTableauToFoundation, from - 'n', to - 'b');
      else
        *this = PlainMove(kTableauToTableau, from - 'n', to - 'n');
    }
  }

  void Show() const;
  string Encode() const;

  bool operator==(const PlainMove& m) const {
    return type == m.type && from == m.from && to == m.to;
  }

  char type;
  unsigned char from : 4;
  unsigned char to : 4;
};

string ReadSolution(int seed);
vector<PlainMove> DecodeSolution(const string& code);

#endif
