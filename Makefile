SRC_DIR := . src src/vm src/vm/value src/vm/standard src/doc \
src/compiler src/compiler/lexical src/compiler/syntaxic src/compiler/semantic \
src/compiler/value src/compiler/instruction src/util lib benchmark test

BUILD_DIR := $(addprefix build/,$(SRC_DIR))
BUILD_DIR += $(addprefix build/shared/,$(SRC_DIR))
SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
OBJ := $(patsubst %.cpp,build/%.o,$(SRC))
OBJ_LIB := $(patsubst %.cpp,build/shared/%.o,$(SRC))

FLAGS := -std=c++14 -O2 -g3 -Wall -Wextra -fprofile-arcs -ftest-coverage
LIBS := -ljit

.PHONY: test

all: makedirs leekscript

fast:
	make -j8

test:
	@build/leekscript -test

build/%.o: %.cpp
	g++ -c $(FLAGS) -o "$@" "$<"
	
build/shared/%.o: %.cpp
	g++ -c $(FLAGS) -fPIC -o "$@" "$<"
	
makedirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

leekscript: $(OBJ)
	g++ $(FLAGS) -o build/leekscript $(OBJ) $(LIBS)
	@echo "---------------"
	@echo "Build finished!"
	@echo "---------------"
	
lib: makedirs $(OBJ_LIB) 
	g++ $(FLAGS) -shared -o build/libleekscript.so $(OBJ_LIB) $(LIBS)
	@echo "-----------------------"
	@echo "Library build finished!"
	@echo "-----------------------"
	
install:
	cp build/libleekscript.so /usr/lib/
	@find -iregex '.*\.\(hpp\|h\|tcc\)' | cpio -updm /usr/include/leekscript/
	@echo "Library installed!"
	
clean:
	rm -rf build
	@echo "----------------"
	@echo "Project cleaned."
	@echo "----------------"

travis:
	docker build -t leekscript .
	docker run -e COVERALLS_REPO_TOKEN="$$COVERALLS_REPO_TOKEN" leekscript /bin/bash -c "printenv; cd leekscript; make -j4 && make test && cpp-coveralls --gcov-options='-r'"
	
cloc:
	cloc . --exclude-dir=.git,lib,build
