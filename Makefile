CXX = g++
CXXFLAGS = -Wall -std=c++14 -O3 -pthread
LDLIBS = -lcrypto
HEADERS = $(shell find . -name "*.hpp")
OBJDIR = obj
OBJS = $(OBJDIR)/main.o $(OBJDIR)/dictionary.o $(OBJDIR)/translate.o $(OBJDIR)/serialize.o $(OBJDIR)/polynet.o $(OBJDIR)/polyweb.o $(OBJDIR)/polyweb_string.o
PREFIX = /usr/local
TARGET = declengine

$(TARGET): $(OBJS) whitakers-words/bin/words
	$(CXX) $(OBJS) $(CXXFLAGS) $(LDLIBS) -o $@

$(OBJDIR)/main.o: main.cpp $(HEADERS)
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/dictionary.o: dictionary.cpp dictionary.hpp words.hpp Polyweb/string.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/translate.o: translate.cpp words.hpp json.hpp Polyweb/string.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/serialize.o: serialize.cpp words.hpp json.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

whitakers-words/bin/words:
	cd whitakers-words && $(MAKE)

$(OBJDIR)/polynet.o: Polyweb/Polynet/polynet.cpp Polyweb/Polynet/polynet.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/polyweb.o: Polyweb/polyweb.cpp Polyweb/polyweb.hpp Polyweb/Polynet/polynet.hpp Polyweb/string.hpp Polyweb/threadpool.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/polyweb_string.o: Polyweb/string.cpp Polyweb/string.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

.PHONY: clean

clean:
	rm -rf $(TARGET) $(OBJDIR)
