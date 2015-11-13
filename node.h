#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
using namespace std;

#include "bit_stream.h"
#include "foundation.h"
#include "list.h"
#include "move.h"
#include "stock.h"
#include "tableau.h"

static constexpr int kCardsPerDeck = 52;
static constexpr int kTotalCards = 156;
static constexpr int kMinMoves = kTotalCards + kInitStockCards;
static constexpr int kMaxMoves = 300;

class Node;

class Pool {
 public:
  Node* New();
  Node* New(const Node& node);
  void Delete(Node* node);

 private:
  Node* Allocate();
  void Free(Node* node);

  Node* head_ = nullptr;
};

class Node {
 public:
  typedef BitStream<736> Moves;

  Node(int seed) : stock_(kInitStockCards) {
    vector<Card> cards(kTotalCards);
    for (int i = 0; i < kTotalCards; ++i) cards[i] = Card(i % kCardsPerDeck);

    srand(seed);
    for (int i = 0; i < kTotalCards * 3; ++i)
      std::swap(cards[rand() % kTotalCards], cards[rand() % kTotalCards]);

    set_cards(cards);
  }

  Node() {}
  void set_cards(const vector<Card>& cards) {
    assert(cards.size() == kTotalCards);

    Stock::set_init_stock(vector<Card>(&cards[0], &cards[kInitStockCards]));
    new (&stock_) Stock(kInitStockCards);

    int k = kInitStockCards;
    for (int i = 0; i < 13; ++i) {
      for (int j = 0; j <= i; ++j) Tableau::init_tableau_[i][j] = cards[k++];
      tableau_[i].set_cards(i, i + 1);
    }
    assert(k == kTotalCards);

    cards_unflipped_ = kTotalCards - kInitStockCards - 13;
    waste_chaos_ = 0;
    tableau_chaos_ = MeasureTableauChaos();
    moves_performed_ = 0;
    moves_estimated_ = stock_.stock_size() + kTotalCards;
  }

  Node(const Node& node) {
    int copy_size = reinterpret_cast<char*>(&moves_) + node.moves_.copy_size() -
                    reinterpret_cast<char*>(this);
    memcpy(this, &node, copy_size);
  }

  int MeasureTableauChaos() const {
    int chaos = 0;
    for (int t = 0; t < 13; ++t) chaos += tableau_[t].MeasureChaos();
    return chaos;
  }

  bool AllowWasteToFoundation() const {
    return last_move_.type != kTableauToTableau;
  }
  bool AllowWasteToFoundation(int f) const {
    return !((last_move_.type == kTableauToFoundation && f != last_move_.to));
  }
  bool AllowWasteToTableau(int t) const {
    return !(last_move_.type == kTableauToTableau && t != last_move_.from &&
             t != last_move_.to);
  }
  bool AllowTableau() const { return last_move_.type != kFlipStock; }
  bool AllowTableau(int t) const {
    return !((last_move_.type == kWasteToTableau ||
              last_move_.type == kTableauToTableau) &&
             t == last_move_.to);
  }
  bool AllowTableauToFoundation(int t, int f) const {
    return !(last_move_.type == kTableauToFoundation && last_move_.to != f &&
             last_move_.from > t) &&
           !(last_move_.type == kTableauToTableau && t != last_move_.from &&
             t != last_move_.to);
  }
  bool AllowTableauToTableau(int t1, int t2) const {
    return !(last_move_.type == kTableauToTableau &&
             last_move_.from + last_move_.to > t1 + t2);
  }
  bool AllowFoundationToTableau() const { return false; }

  List<Node> Expand(Pool* pool) const {
    List<Node> new_nodes;
    if (stock_.waste_size() > 0) {
      auto card = stock_.Top();
      if (AllowWasteToFoundation()) {
        auto f = FindFoundation(card);
        if (f >= 0 && AllowWasteToFoundation(f)) {
          new_nodes.Append(pool->New(*this)->WasteToFoundation(f));
          if (card.rank() <= R2) {
            EncodeMoves(new_nodes);
            return new_nodes;
          }
        }
      }
      for (int i = 0; i < 13; ++i) {
        if (tableau_[i].Accepting(card) && AllowWasteToTableau(i)) {
          new_nodes.Append(pool->New(*this)->WasteToTableau(i));
          break;
        }
      }
    }
    if (stock_.stock_size() > 0) {
      auto new_node = pool->New(*this)->FlipStock();
      new_nodes.Append(new_node);
      // The first move is always to flip the stock.
      if (moves_performed_ == 0) {
        new_node->last_move_ = PlainMove(kNone, 0, 0);
        EncodeMoves(new_nodes);
        return new_nodes;
      }
    }
    if (AllowTableau()) {
      for (int i = 0; i < 13; ++i) {
        if (tableau_[i].empty()) continue;
        if (!AllowTableau(i)) continue;

        if (last_move_.type != kWasteToTableau) {
          auto card = tableau_[i].Top();
          auto f = FindFoundation(card);
          if (f >= 0 && AllowTableauToFoundation(i, f))
            new_nodes.Append(pool->New(*this)->TableauToFoundation(i, f));
        }

        for (int j = 0; j < 13; ++j) {
          if (j == i) continue;
          // Don't move K stack to empty tableau.
          if (tableau_[j].empty() && tableau_[i].unflipped_size() == 0 &&
              tableau_[i].card(0).rank() == KING)
            continue;
          if (!AllowTableauToTableau(i, j)) continue;

          auto count = tableau_[i].CountMovable(tableau_[j]);
          if (count > 0 && count == tableau_[i].flipped_size()) {
            new_nodes.Append(pool->New(*this)->TableauToTableau(i, j));
            break;
          }
        }
      }
    }
    if (AllowFoundationToTableau()) {
      for (int f = 0; f < 12; ++f) {
        if (foundation_[f].empty()) continue;

        for (int t = 0; t < 13; ++t) {
          if (tableau_[t].Accepting(foundation_[f].Top(f & 3)))
            new_nodes.Append(pool->New(*this)->FoundationToTableau(f, t));
        }
      }
    }
    EncodeMoves(new_nodes);
    return new_nodes;
  }

  int FindFoundation(Card card) const {
    auto suit = card.suit();
    for (int i = suit; i < 12; i += 4)
      if (foundation_[i].Accepting(card)) return i;
    return -1;
  }

  Node* FlipStock() {
    auto old_stock_size = stock_.stock_size();
    auto old_waste_size = stock_.waste_size();
    if (stock_.waste_size() > 0) {
      auto old_top = stock_.Top();
      stock_.Flip();
      waste_chaos_ += std::max(0, stock_.Top().rank() - old_top.rank());
    } else {
      stock_.Flip();
    }
    moves_estimated_ += ((stock_.stock_size() - old_stock_size) << 1) +
                        (stock_.waste_size() - old_waste_size);
    last_move_ = PlainMove(kFlipStock, 0, 0);
    moves_performed_++;
    return this;
  }

  Node* WasteToFoundation(int f) {
    auto card = stock_.Top();
    stock_.Pop();
    if (stock_.waste_size() > 0) {
      waste_chaos_ -= std::max(0, card.rank() - stock_.Top().rank());
      assert(waste_chaos_ >= 0);
    }
    foundation_[f].Push(card);
    moves_estimated_ -= 1;
    last_move_ = PlainMove(kWasteToFoundation, 0, f);
    moves_performed_++;
    return this;
  }

  Node* WasteToTableau(int t) {
    auto card = stock_.Top();
    stock_.Pop();
    if (stock_.waste_size() > 0) {
      waste_chaos_ -= std::max(0, card.rank() - stock_.Top().rank());
      assert(waste_chaos_ >= 0);
    }
    tableau_[t].Push(card);
    moves_estimated_ += 0;
    last_move_ = PlainMove(kWasteToTableau, 0, t);
    moves_performed_++;
    return this;
  }

  Node* TableauToFoundation(int t, int f) {
    auto card = tableau_[t].Top();
    bool flipped = tableau_[t].Pop();
    if (flipped) {
      tableau_chaos_ -= tableau_[t].ChaosAt(tableau_[t].size());
      assert(tableau_chaos_ >= 0);
    }
    foundation_[f].Push(card);
    cards_unflipped_ -= flipped;
    moves_estimated_ -= 1;
    last_move_ = PlainMove(kTableauToFoundation, t, f);
    moves_performed_++;
    return this;
  }

  Node* TableauToTableau(int s, int t) {
    bool flipped = tableau_[s].Move(&tableau_[t]);
    if (flipped) {
      tableau_chaos_ -= tableau_[s].ChaosAt(tableau_[s].size());
      assert(tableau_chaos_ >= 0);
    }
    cards_unflipped_ -= flipped;
    moves_estimated_ += 0;
    last_move_ = PlainMove(kTableauToTableau, s, t);
    moves_performed_++;
    return this;
  }

  Node* FoundationToTableau(int f, int t) {
    auto card = foundation_[f].Top(f & 3);
    foundation_[f].Pop();
    tableau_[t].Push(card);
    moves_estimated_ += 1;
    last_move_ = PlainMove(kFoundationToTableau, f, t);
    moves_performed_++;
    return this;
  }

  string CompleteSolution();

  void EncodeMoves(const List<Node>& new_nodes) const {
    int index = 0;
    for (auto* node : new_nodes) node->moves_.Write(index++, new_nodes.size());
  }

  void PlayMoves(const vector<PlainMove>& moves);
  void ShowSummary() const;
  void Show() const;

  const Stock& stock() const { return stock_; }
  const Foundation& foundation(int f) const { return foundation_[f]; }
  const Tableau& tableau(int t) const { return tableau_[t]; }
  const Moves& moves() const { return moves_; }
  const PlainMove& last_move() const { return last_move_; }

  int bin() const { return cost(); }
  int min_total_moves() const { return moves_performed_ + moves_estimated_; }
  int moves_performed() const { return moves_performed_; }
  void set_moves_performed(int moves) { moves_performed_ = moves; }
  int moves_estimated() const { return moves_estimated_; }
  int cost() const {
    return (moves_estimated_ << 3) + moves_estimated_ + (waste_chaos_ << 1) +
           waste_chaos_ + tableau_chaos_;
  }
  int cards_unflipped() const { return cards_unflipped_; }

  bool operator==(const Node& n) const {
    if (stock_.stock_size() != n.stock_.stock_size() ||
        stock_.waste_size() != n.stock_.waste_size())
      return false;

    for (int f = 0; f < 12; ++f)
      if (foundation_[f].size() != n.foundation_[f].size()) return false;

    for (int t = 0; t < 13; ++t)
      if (tableau_[t].unflipped_size() != n.tableau_[t].unflipped_size() ||
          tableau_[t].size() != n.tableau_[t].size())
        return false;

    if (stock_ != n.stock_) return false;

    for (int t = 0; t < 13; ++t)
      if (tableau_[t] != n.tableau_[t]) return false;

    return true;
  }
  bool operator!=(const Node& n) const { return !(*this == n); }

  unsigned hash() const { return hash_; }

  void ComputeHash() {
    hash_ = stock_rand_[stock_.stock_size()];
    hash_ += waste_rand_[stock_.waste_size()];
    if (stock_.waste_size()) hash_ += waste_top_rand_[stock_.Top().card()];
    for (int i = 0; i < 13; ++i) {
      hash_ += tableau_unflipped_rand_[i][tableau_[i].unflipped_size()];
      hash_ += tableau_flipped_rand_[i][tableau_[i].flipped_size()];
      if (tableau_[i].flipped_size())
        hash_ += tableau_top_rand_[i][tableau_[i].Top().card()];
    }
  }

  static const Node& goal() { return goal_; }
  static void Initialize();
  static void InitializeHashRand(int count, vector<unsigned>* rand);

 private:
  Stock stock_;
  Foundation foundation_[12];
  Tableau tableau_[13];

  unsigned hash_;
  short tableau_chaos_;
  short waste_chaos_;
  short cards_unflipped_;
  short moves_performed_;
  short moves_estimated_;
  PlainMove last_move_;
  Moves moves_;

  friend class HashTable;
  Node* prev_;
  Node* next_;

  friend class Pool;
  friend class List<Node>;
  friend class Bucket;
  Node* after_;

  static vector<unsigned> stock_rand_;
  static vector<unsigned> waste_rand_;
  static vector<unsigned> waste_top_rand_;
  static vector<vector<unsigned>> tableau_unflipped_rand_;
  static vector<vector<unsigned>> tableau_flipped_rand_;
  static vector<vector<unsigned>> tableau_top_rand_;

  static Node* pool_;
  static Node goal_;
};

// TODO(hanhong): Can't return a ScopedNode!
class ScopedNode {
 public:
  ScopedNode(Pool* pool) : pool_(pool), node_(nullptr) {}
  ScopedNode(Pool* pool, Node* node) : pool_(pool), node_(node) {}
  ~ScopedNode() { reset(); }

  Node* operator->() const { return node_; }
  const Node& operator*() const { return *node_; }
  operator bool() const { return node_ != nullptr; }

  void reset() { pool_->Delete(node_); }
  void reset(Node* node) {
    reset();
    node_ = node;
  }
  Node* release() {
    auto old_node = node_;
    node_ = nullptr;
    return old_node;
  }

 private:
  Pool* const pool_;
  Node* node_;
};

#endif
