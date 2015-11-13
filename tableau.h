#ifndef TABLEAU_H
#define TABLEAU_H

#include <assert.h>
#include <vector>
using namespace std;

#include "card.h"

class Tableau {
 public:
  Tableau() : index_(0), unflipped_size_(0), flipped_size_(0), stack_(0) {}
  Tableau(int index, int size) { set_cards(index, size); }

  void set_cards(int index, int size) {
    index_ = index;
    unflipped_size_ = size - 1;
    flipped_size_ = 1;
    stack_ = 0;
    top_ = init_tableau_[index][size - 1];
  }

  Card Top() const { return top_; }

  void Push(Card card) {
    assert(Accepting(card));
    assert(flipped_size_ < 13);
    if (flipped_size_++ > 0) PushTopToStack();
    top_ = card;
  }

  bool Pop() {
    assert(flipped_size_ > 0);
    if (flipped_size_-- > 1) {
      bool is_major = stack_ & 1;
      int color = !top_.color();
      int suit = is_major ? color : 3 - color;
      top_ = Card(suit, top_.rank() + 1);
      stack_ >>= 1;
    }
    return MaybeFlip();
  }

  bool Move(Tableau* to) {
    auto count = CountMovable(*to);
    assert(count == flipped_size_);

    if (!to->empty()) to->PushTopToStack();
    if (count > 1) to->stack_ = (to->stack_ << (count - 1)) | stack_;
    to->top_ = top_;
    to->flipped_size_ += count;

    stack_ = 0;
    flipped_size_ -= count;
    return MaybeFlip();
  }

  int CountMovable(const Tableau& target) const {
    if (empty()) return 0;
    if (target.empty()) {
      if (top_.rank() + flipped_size_ - 1 != KING) return 0;
      return flipped_size_;
    } else {
      auto lead = target.Top();
      auto top = Top();
      int rank_diff = lead.rank() - top.rank();
      if (rank_diff <= 0) return 0;
      if (flipped_size_ < rank_diff) return 0;
      if ((rank_diff & 1) == (top.color() == lead.color())) return 0;
      return rank_diff;
    }
  }

  bool Accepting(Card card) const {
    return empty() ? card.rank() == KING : card.IsBelow(Top());
  }

  bool empty() const { return !size(); }
  int size() const { return unflipped_size_ + flipped_size_; }
  int unflipped_size() const { return unflipped_size_; }
  int flipped_size() const { return flipped_size_; }

  Card card(int i) const {
    assert(i < size());
    if (i < unflipped_size_) return init_tableau_[index_][i];
    if (i == size() - 1) return top_;

    int depth = size() - 2 - i;
    bool is_major = stack_ & (1 << depth);
    int color = (depth & 1) ? top_.color() : !top_.color();
    int suit = is_major ? color : 3 - color;
    return Card(suit, top_.rank() + depth + 1);
  }

  int ChaosAt(int i) const {
    assert(0 < i && i <= index_);
    return std::max(0, init_tableau_[index_][i].rank() -
                           init_tableau_[index_][i - 1].rank());
  }

  int MeasureChaos() const {
    int chaos = 0;
    for (int i = 1; i < size(); ++i) {
      chaos += ChaosAt(i);
    }
    return chaos;
  }

  bool operator==(const Tableau& t) const {
    if (unflipped_size() != t.unflipped_size() ||
        flipped_size() != t.flipped_size())
      return false;
    if (flipped_size() == 0) return true;
    if (top_ != t.top_) return false;
    return stack_ == t.stack_;
    // unsigned mask = (1 << (flipped_size() - 1)) - 1;
    // return (stack_ & mask) == (t.stack_ & mask);
  }
  bool operator!=(const Tableau& t) const { return !(*this == t); }

  static Card init_tableau_[13][13];

 private:
  void PushTopToStack() { stack_ = (stack_ << 1) | top_.IsMajor(); }

  bool MaybeFlip() {
    if (flipped_size_ == 0 && unflipped_size_ > 0) {
      --unflipped_size_;
      ++flipped_size_;
      top_ = init_tableau_[index_][unflipped_size_];
      return true;
    } else
      return false;
  }

  unsigned index_ : 4;
  unsigned unflipped_size_ : 4;
  unsigned flipped_size_ : 4;
  unsigned stack_ : 12;
  Card top_;
};

#endif
