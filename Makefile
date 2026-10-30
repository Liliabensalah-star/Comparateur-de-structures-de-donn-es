# Makefile — Comparateur de structures de données (Groupe 8, ESGI 2A3)
#
#   make            compile le programme et tous les plugins
#   make run        compile puis lance le programme
#   make valgrind   lance le programme sous valgrind (fuites mémoire)
#   make plugins    compile seulement les plugins .so
#   make clean      supprime tout ce qui a été compilé
#
# Ajouter un fichier dans src/ ou une structure dans plugins/ :
# rien à modifier ici, il est pris en compte automatiquement.

CC      := gcc
CFLAGS  := -Wall -Wextra -std=gnu11 -g -Iinclude
DEPFLAGS = -MMD -MP

# SDL : options trouvées automatiquement par pkg-config.
# SDL2 (paquet libsdl2-dev) par défaut ; pour SDL 1.2 : make SDL_PKG=sdl
SDL_PKG    ?= sdl2
SDL_CFLAGS := $(shell pkg-config --cflags $(SDL_PKG) 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs $(SDL_PKG) 2>/dev/null)
ifeq ($(SDL_LIBS),)
$(warning SDL introuvable ($(SDL_PKG)) : installer libsdl2-dev, ou lancer make SDL_PKG=sdl pour SDL 1.2)
endif

LDLIBS  := -ldl -lm $(SDL_LIBS)

# Programme principal : tous les .c de src/, compilés dans build/
BIN     := comparateur
SRC     := $(wildcard src/*.c)
OBJ     := $(patsubst src/%.c,build/%.o,$(SRC))
DEP     := $(OBJ:.o=.d)

# Plugins : plugins/xxx.c -> plugins/xxx.so (sans SDL : une structure ne dessine pas)
PLUGINS := $(patsubst %.c,%.so,$(wildcard plugins/*.c))

.PHONY: all plugins run valgrind clean

all: $(BIN) $(PLUGINS)

plugins: $(PLUGINS)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(DEPFLAGS) -c $< -o $@

plugins/%.so: plugins/%.c include/structure.h
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@

build:
	mkdir -p build resultats

run: all
	./$(BIN)

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(BIN)

clean:
	rm -rf build $(BIN) plugins/*.so

# Recompile un .c quand un .h qu'il inclut change
-include $(DEP)
