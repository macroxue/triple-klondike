# triple-klondike
A solver for Minimal Triple Klondike contest at http://hardpuzzles.free.fr/.

## The Algorithm
The program applies simple Beam Search with a few optimizations.

a) It has a compact state with all its move history in about 200 bytes, so 16GB would allow 80 million states. 
On average, 3 bits are needed to record one move. A move doesn't have to be in the form (source, destination). 
It can be just an index to all moves allowed from a state and works well when the branching factor is small.

b) The program does not have any look-ahead or roll-out. It simply evaluates the state as is.
 * lowerbound = number of stock cards * 2 + number of waste cards + number of tableau cards
 * chaos(r_1, r_2, ..., r_n) = max(0, r_1 - r_2) + max(0, r_2 - r_3) + ... + max(0, r_{n-1} - r_n), where r_i are ranks of cards.
 * heuristic cost = w_1 * lowerbound + w_2  * chaos(waste) + w_3 * Sum_i chaos(tableau_i), where weights w_1 = 9, w_2 = 3 and w_3 = 1 are chosen empirically.

c) It prunes the search tree, observing independency between moves. For example, a stock-to-waste move and 
a tableau-to-tableau move are independent, so it forbids stock-to-waste after tableau-to-tableau. 
The rules are quite complicated when all move types are considered.

d) It enforces restrictions to simplify the search:
 * Foundation-to-tableau moves are forbidden.
 * A tableau-to-tableau move is allowed only if it reveals a card or it empties a tableau.
 * Stock cards are flipped through for one round only.

e) The program also enables multi-threaded processing for a single deal, so the search can be much faster on 
a multi-core system.

f) It applies Beam Search on existing solutions. For example, it plays the first N moves of a solution and 
starts a new search from there. A lot of times the initial solution can be significantly improved. 
However, this requires much more compute power since the range of N is quite large. For this reason, 
it simply chooses N in { 20, 40, 60, ... }.

## How to Run

Just make on a Linux system with GCC installed. C++0x or above is required. 

Then try the following to solve deal #12 with a beam size of 2^13 and 2 threads. There are 100 deals in total.
```
./solver 12 13 2
```
