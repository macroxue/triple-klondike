#include <assert.h>
#include <stdio.h>
#include "node.h"

void test_play() {
  Node n(1);
  n.Show();

  n.TableauToFoundation(0, 0);
  n.TableauToFoundation(2, 3);
  n.TableauToFoundation(9, 2);
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.WasteToFoundation(6);
  n.WasteToFoundation(1);
  n.TableauToTableau(3, 7);
  n.TableauToTableau(10, 4);
  n.TableauToTableau(1, 4);
  n.TableauToTableau(4, 0);
  n.TableauToTableau(4, 7);
  n.TableauToTableau(4, 1);
  n.TableauToFoundation(4, 3);
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.WasteToFoundation(1);
  n.TableauToFoundation(8, 1);
  n.TableauToTableau(8, 0);
  n.TableauToTableau(1, 5);
  n.TableauToTableau(7, 1);
  n.WasteToTableau(4);
  n.WasteToTableau(12);
  n.TableauToTableau(5, 12);
  n.TableauToTableau(8, 5);
  n.TableauToTableau(7, 5);
  n.TableauToTableau(7, 10);
  n.TableauToTableau(7, 11);
  n.TableauToTableau(7, 0);
  n.TableauToFoundation(7, 4);
  n.TableauToTableau(2, 10);
  n.WasteToTableau(2);
  n.TableauToTableau(6, 2);
  n.TableauToTableau(7, 9);
  n.TableauToTableau(6, 7);
  n.TableauToTableau(2, 12);
  n.TableauToTableau(12, 5);
  n.TableauToTableau(5, 2);
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.WasteToFoundation(5);
  n.WasteToTableau(0);
  n.TableauToTableau(10, 0);
  n.TableauToTableau(11, 10);
  n.TableauToTableau(11, 1);
  n.TableauToTableau(8, 11);
  n.TableauToFoundation(8, 1);
  n.TableauToFoundation(8, 0);
  n.TableauToTableau(8, 4);
  n.TableauToTableau(8, 0);
  n.TableauToTableau(8, 1);
  n.TableauToTableau(12, 1);
  n.TableauToFoundation(12, 0);
  n.TableauToTableau(11, 7);
  n.TableauToTableau(3, 9);
  n.TableauToTableau(12, 11);
  n.TableauToTableau(3, 12);
  n.TableauToTableau(10, 11);
  n.TableauToTableau(11, 10);
  n.TableauToTableau(11, 8);
  n.TableauToFoundation(11, 7);
  n.TableauToFoundation(10, 7);
  n.TableauToTableau(11, 4);
  n.TableauToFoundation(11, 11);
  n.TableauToTableau(6, 3);
  n.TableauToTableau(6, 12);
  n.TableauToFoundation(0, 0);
  n.TableauToTableau(3, 0);
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.WasteToTableau(12);
  n.TableauToTableau(5, 12);
  n.TableauToTableau(10, 5);
  n.TableauToFoundation(10, 1);
  n.TableauToTableau(10, 9);
  n.WasteToFoundation(0);
  n.WasteToTableau(9);
  n.FlipStock();
  n.FlipStock();
  n.WasteToTableau(11);
  n.FlipStock();
  n.WasteToFoundation(11);
  n.FlipStock();
  n.WasteToTableau(8);
  n.FlipStock();
  n.WasteToFoundation(3);
  n.FlipStock();
  n.WasteToTableau(7);
  n.TableauToTableau(6, 10);
  n.TableauToTableau(10, 7);
  n.TableauToFoundation(6, 8);
  n.TableauToTableau(6, 7);
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.FlipStock();
  n.WasteToTableau(3);
  n.TableauToTableau(12, 3);

  n.Show();
}

void test_duplicate() {
  Node m(1);
  Node n(1);
  assert(m == n);

  m.FlipStock();
  assert(m != n);
  n.FlipStock();
  assert(m == n);

  m.TableauToFoundation(0, 0);
  assert(m != n);
  n.TableauToFoundation(0, 0);
  assert(m == n);
}

int main() {
  printf("sizeof(Node) = %lu\n", sizeof(Node));
  printf("sizeof(Node::Moves) = %lu\n", sizeof(Node::Moves));
  printf("sizeof(Move) = %lu\n", sizeof(Move));
  printf("sizeof(PlainMove) = %lu\n", sizeof(PlainMove));

  test_duplicate();
  test_play();

  puts("PASSED");
  return 0;
}
