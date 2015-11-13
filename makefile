TESTS=card_test stock_test foundation_test tableau_test bit_stream_test node_test

BINARIES=solver visualizer

all: $(TESTS) $(BINARIES)

clean:
	rm -f $(TESTS) $(BINARIES)

card_test: card_test.cc card.h
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^)
	./$@

stock_test: stock_test.cc stock.cc stock.h card.h
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^)
	./$@

foundation_test: foundation_test.cc foundation.h card.h
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^)
	./$@

tableau_test: tableau_test.cc tableau.cc tableau.h card.h
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^)
	./$@

bit_stream_test: bit_stream_test.cc bit_stream.h
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^)
	./$@

NODE_DEPS=node.cc node.h list.h move.cc move.h \
	  tableau.cc tableau.h foundation.h stock.cc stock.h card.h

node_test: node_test.cc $(NODE_DEPS)
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^)
	./$@

SOLVER_DEPS=solver.cc deals.cc deals.h bucket.h hash_table.h $(NODE_DEPS)

solver: $(SOLVER_DEPS)
	g++ -O3 -std=c++0x -o $@ $(filter %.cc,$^) -pthread

solver.g: $(SOLVER_DEPS)
	g++ -g -std=c++0x -o $@ $(filter %.cc,$^) -pthread

solver.p: $(SOLVER_DEPS)
	g++ -O3 -std=c++0x -fprofile-generate -o $@ $(filter %.cc,$^) -pthread -DNDEBUG
	time ./$@ 12 12

solver.q: $(SOLVER_DEPS)
	g++ -O3 -std=c++0x -fprofile-use -o $@ $(filter %.cc,$^) -pthread -DNDEBUG
	time ./$@ 12 12

visualizer: visualizer.cc node.cc node.h list.h move.cc move.h deals.cc deals.h \
	          tableau.cc tableau.h stock.cc stock.h card.h
	g++ -O3 -std=c++0x -o $@ $(filter %.cc,$^)
