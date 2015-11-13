#include "node.h"

Node* Node::pool_;
Node Node::goal_;

vector<unsigned> Node::stock_rand_;
vector<unsigned> Node::waste_rand_;
vector<unsigned> Node::waste_top_rand_;
vector<vector<unsigned>> Node::tableau_unflipped_rand_;
vector<vector<unsigned>> Node::tableau_flipped_rand_;
vector<vector<unsigned>> Node::tableau_top_rand_;

void Node::Initialize() {
  InitializeHashRand(kInitStockCards + 1, &stock_rand_);
  InitializeHashRand(kInitStockCards + 1, &waste_rand_);
  InitializeHashRand(kCardsPerDeck, &waste_top_rand_);

  tableau_unflipped_rand_.resize(13);
  tableau_flipped_rand_.resize(13);
  tableau_top_rand_.resize(13);
  for (int i = 0; i < 13; ++i) {
    InitializeHashRand(13, &tableau_unflipped_rand_[i]);
    InitializeHashRand(14, &tableau_flipped_rand_[i]);
    InitializeHashRand(kCardsPerDeck, &tableau_top_rand_[i]);
  }

  pool_ = nullptr;

  for (int i = 0; i < 12; ++i) {
    int suit = i % 4;
    for (int rank = ACE; rank <= KING; ++rank)
      goal_.foundation_[i].Push(Card(suit, rank));
  }
  goal_.ComputeHash();
}

void Node::InitializeHashRand(int count, vector<unsigned>* rand) {
  for (int i = 0; i < count; ++i) rand->push_back(lrand48());
}

void Node::ShowSummary() const {
  printf("\tMoves: %d  Estimate: %d  Cost: %d  Hash: %u  WC: %d  TC: %d\n",
         moves_performed_, moves_estimated_, cost(), hash_, waste_chaos_,
         tableau_chaos_);
}

void Node::Show() const {
  last_move_.Show();
  ShowSummary();
  stock_.Show();
  printf("Foundations: ");
  for (int i = 0; i < 12; ++i)
    if (foundation_[i].empty())
      printf("[%c ] ", "SHDC"[i & 3]);
    else
      printf("[%s] ", foundation_[i].Top(i & 3).ToString());
  puts("");
  for (int i = 0; i < 13; ++i) {
    printf("Tableau %2d: ", i);
    for (int j = 0; j < tableau_[i].unflipped_size(); ++j)
      printf("%s ", tableau_[i].card(j).ToString());
    printf("| ");
    for (int j = tableau_[i].unflipped_size(); j < tableau_[i].size(); ++j)
      printf("%s ", tableau_[i].card(j).ToString());
    puts("");
  }
}

void Node::PlayMoves(const vector<PlainMove>& moves) {
  for (const auto& move : moves) {
    switch (move.type) {
      case kNone:
        break;
      case kFlipStock:
        FlipStock();
        break;
      case kWasteToFoundation:
        WasteToFoundation(move.to);
        break;
      case kWasteToTableau:
        WasteToTableau(move.to);
        break;
      case kTableauToFoundation:
        TableauToFoundation(move.from, move.to);
        break;
      case kTableauToTableau:
        TableauToTableau(move.from, move.to);
        break;
      case kFoundationToTableau:
        FoundationToTableau(move.from, move.to);
        break;
    }
  }
}

string Node::CompleteSolution() {
  assert(cards_unflipped_ == 0);
  assert(stock_.size() == 0);
  string code;
  bool all_tableau_empty;
  do {
    all_tableau_empty = true;
    for (int i = 0; i < 13; ++i) {
      while (!tableau_[i].empty()) {
        int f = FindFoundation(tableau_[i].Top());
        if (f >= 0) {
          TableauToFoundation(i, f);
          code += last_move_.Encode();
        } else
          break;
      }
      all_tableau_empty &= tableau_[i].empty();
    }
  } while (!all_tableau_empty);
  return code;
}

Node* Pool::New(const Node& node) { return new (Allocate()) Node(node); }
Node* Pool::New() { return new (Allocate()) Node(); }
void Pool::Delete(Node* node) { Free(node); }

Node* Pool::Allocate() {
  if (!head_) {
    auto nodes = new Node[256];
    for (int i = 0; i < 256; ++i) Free(&nodes[i]);
  }
  auto old_head = head_;
  head_ = head_->after_;
  return old_head;
}

void Pool::Free(Node* node) {
  if (node) {
    node->after_ = head_;
    head_ = node;
  }
}
