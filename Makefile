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

HEADERS = src/strbuf.h src/csv.h

MOVBASE_OBJS = $(OBJ_DIR)/main.o \
			   $(OBJ_DIR)/strbuf.o \
			   $(OBJ_DIR)/csv.o

TEST_CSV_OBJS = $(OBJ_DIR)/csv.o \
			   	$(OBJ_DIR)/strbuf.o \
			   	$(OBJ_DIR)/test/csv.o

$(OBJ_DIR)/%.o: src/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) -c $< $(CFLAGS) -o $@

movbase: $(MOVBASE_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

test/csv: $(TEST_CSV_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

clean:
	$(RM) $(BASE_BUILD_DIR)
