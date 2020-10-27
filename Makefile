# Release or debug or sanitize
PROFILE = RELEASE

BASE_BUILD_DIR = build
BUILD_DIR_DEBUG = $(BASE_BUILD_DIR)/debug
BUILD_DIR_RELEASE = $(BASE_BUILD_DIR)/release
BUILD_DIR_SANITIZE = $(BASE_BUILD_DIR)/sanitize
BUILD_DIR = $(BUILD_DIR_$(PROFILE))

OBJ_DIR = $(BUILD_DIR)/obj

SANITIZERS = -fsanitize=address \
			 -fsanitize=pointer-compare \
			 -fsanitize=pointer-subtract \
			 -fsanitize=undefined \
			 -fsanitize=shift \
			 -fsanitize=integer-divide-by-zero \
			 -fsanitize=null \
			 -fsanitize=return \
			 -fsanitize=signed-integer-overflow \
			 -fsanitize=bounds-strict \
			 -fsanitize=alignment \
			 -fsanitize=float-divide-by-zero \
			 -fsanitize=float-cast-overflow \
			 -fsanitize=enum \
			 -fsanitize=bool \
			 -fsanitize=object-size \
			 -fsanitize=leak

BASE_CFLAGS = -Wall
CFLAGS_DEBUG = $(BASE_CFLAGS) -g
CFLAGS_RELEASE = $(BASE_CFLAGS) -O3
CFLAGS_SANITIZE = $(BASE_CFLAGS) -g  $(SANITIZERS)

CFLAGS = $(CFLAGS_$(PROFILE))

BASE_LDFLAGS = -lm
LDFLAGS_DEBUG = $(BASE_LDFLAGS) -g
LDFLAGS_RELEASE =  $(BASE_LDFLAGS) -O3
LDFLAGS_SANITIZE = $(BASE_LDFLAGS) -g $(SANITIZERS)
LDFLAGS = $(LDFLAGS_$(PROFILE))

HEADERS = src/error.h \
		  src/alloc.h \
		  src/strref.h \
		  src/strbuf.h \
		  src/io.h \
		  src/id/def.h \
		  src/id.h \
		  src/csv.h \
		  src/csv/movie.h \
		  src/trie/branch.h \
		  src/trie/iter.h \
		  src/trie.h \
		  src/prime.h \
		  src/movies.h \
		  src/shell.h

MOVIEDB_OBJS = $(OBJ_DIR)/main.o \
			   $(OBJ_DIR)/error.o \
			   $(OBJ_DIR)/alloc.o \
			   $(OBJ_DIR)/strref.o \
			   $(OBJ_DIR)/strbuf.o \
			   $(OBJ_DIR)/io.o \
			   $(OBJ_DIR)/csv.o \
			   $(OBJ_DIR)/csv/movie.o \
			   $(OBJ_DIR)/trie/branch.o \
			   $(OBJ_DIR)/trie/iter.o \
			   $(OBJ_DIR)/trie.o \
			   $(OBJ_DIR)/id.o \
			   $(OBJ_DIR)/prime.o \
			   $(OBJ_DIR)/movies.o \
			   $(OBJ_DIR)/shell.o

TEST_CSV_OBJS = $(OBJ_DIR)/error.o \
				$(OBJ_DIR)/alloc.o \
				$(OBJ_DIR)/io.o \
				$(OBJ_DIR)/csv.o \
			   	$(OBJ_DIR)/strbuf.o \
			   	$(OBJ_DIR)/test/csv.o

TEST_TRIE_OBJS = $(OBJ_DIR)/error.o \
				 $(OBJ_DIR)/alloc.o \
			   	 $(OBJ_DIR)/strbuf.o \
				 $(OBJ_DIR)/trie/branch.o \
				 $(OBJ_DIR)/trie/iter.o \
			   	 $(OBJ_DIR)/trie.o \
			   	 $(OBJ_DIR)/test/trie.o

TEST_MOVIES_TABLE_OBJS = $(OBJ_DIR)/error.o \
						 $(OBJ_DIR)/alloc.o \
						 $(OBJ_DIR)/strbuf.o \
						 $(OBJ_DIR)/id.o \
						 $(OBJ_DIR)/prime.o \
						 $(OBJ_DIR)/movies.o \
						 $(OBJ_DIR)/test/movies_table.o

$(OBJ_DIR)/%.o: src/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) -c $< $(CFLAGS) -o $@

moviedb: $(MOVIEDB_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

all: moviedb test/csv

test/csv: $(TEST_CSV_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

test/trie: $(TEST_TRIE_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

test/movies_table: $(TEST_MOVIES_TABLE_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

clean:
	$(RM) -r $(BASE_BUILD_DIR)
