# Release or debug
PROFILE = DEBUG

BASE_BUILD_DIR = build
BUILD_DIR_DEBUG = $(BASE_BUILD_DIR)/debug
BUILD_DIR_RELEASE = $(BASE_BUILD_DIR)/release
BUILD_DIR = $(BUILD_DIR_$(PROFILE))

OBJ_DIR = $(BUILD_DIR)/obj

BASE_CFLAGS = -Wall
CFLAGS_DEBUG = $(BASE_CFLAGS) -g
CFLAGS_RELEASE = $(BASE_CFLAGS) -O3
CFLAGS = $(CFLAGS_$(PROFILE))

BASE_LDFLAGS = -lm
LDFLAGS_DEBUG = $(BASE_LDFLAGS) -g
LDFLAGS_RELEASE =  $(BASE_LDFLAGS) -O3
LDFLAGS = $(LDFLAGS_$(PROFILE))

HEADERS = src/strbuf.h src/csv.h

MOVBASE_OBJS = $(OBJ_DIR)/main.o \
			   $(OBJ_DIR)/strbuf.o \
			   $(OBJ_DIR)/csv.o

$(OBJ_DIR)/%.o: src/%.c $(HEADERS)
	mkdir -p $(dir $(OBJ_DIR)/$@)
	$(CC) -c $< $(CFLAGS) -o $(OBJ_DIR)/$@

movbase: $(MOVBASE_OBJS)
	mkdir -p $(dir $(BUILD_DIR)/$@)
	$(CC) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

clean:
	$(RM) $(BASE_BUILD_DIR)
