TARGETS=proxy

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

test: test.cpp
	g++ -o $@ $< -l pthread

testClient: textClient.cpp
	g++ -o $@ $< -l pthread

proxy: proxy.cpp Cache.hpp parser.hpp
	g++ -o $@ $< -Werror -l pthread
