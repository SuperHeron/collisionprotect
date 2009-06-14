PALUDIS_HOOK_NAME=merger_check_post
PALUDIS_HOOK_SONAME=CollisionProtect

SRC=$(wildcard *.cc)
OBJ=$(SRC:.cc=.o)
HEADERS=$(wildcard *.hh *.h)

all: objbindir PALUDIS_HOOK_SONAME.so

PALUDIS_HOOK_SONAME.so: $(OBJ)
	g++ -shared obj/*.o $(LDFLAGS) `pkg-config --libs paludis` -o bin/$(PALUDIS_HOOK_SONAME).so

%.o: %.cc $(HEADERS)
	g++ -std=c++0x -Wall $(CXXFLAGS) `pkg-config --cflags paludis` -fPIC -c $< -o obj/$@

objbindir:
	mkdir -p obj bin

install:
	mkdir -p $(DESTDIR)/usr/share/paludis/hooks/$(PALUDIS_HOOK_NAME)
	cp bin/$(PALUDIS_HOOK_SONAME).so $(DESTDIR)/usr/share/paludis/hooks/$(PALUDIS_HOOK_NAME)/$(PALUDIS_HOOK_SONAME)_$(PALUDIS_HOOK_SUFFIX)

.PHONY: clean mrproper

clean:
	rm -f obj/*.o

mrproper: clean
	rm -f bin/$(PALUDIS_HOOK_SONAME).so
