# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mingcheng/SimpleCompiler

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mingcheng/SimpleCompiler/build

# Include any dependencies generated for this target.
include CMakeFiles/Front.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Front.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Front.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Front.dir/flags.make

CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o: CMakeFiles/Front.dir/flags.make
CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o: ../src/front/abstract_syntax_tree.cpp
CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o: CMakeFiles/Front.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mingcheng/SimpleCompiler/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o -MF CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o.d -o CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o -c /home/mingcheng/SimpleCompiler/src/front/abstract_syntax_tree.cpp

CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mingcheng/SimpleCompiler/src/front/abstract_syntax_tree.cpp > CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.i

CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mingcheng/SimpleCompiler/src/front/abstract_syntax_tree.cpp -o CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.s

CMakeFiles/Front.dir/src/front/lexical.cpp.o: CMakeFiles/Front.dir/flags.make
CMakeFiles/Front.dir/src/front/lexical.cpp.o: ../src/front/lexical.cpp
CMakeFiles/Front.dir/src/front/lexical.cpp.o: CMakeFiles/Front.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mingcheng/SimpleCompiler/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Front.dir/src/front/lexical.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Front.dir/src/front/lexical.cpp.o -MF CMakeFiles/Front.dir/src/front/lexical.cpp.o.d -o CMakeFiles/Front.dir/src/front/lexical.cpp.o -c /home/mingcheng/SimpleCompiler/src/front/lexical.cpp

CMakeFiles/Front.dir/src/front/lexical.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Front.dir/src/front/lexical.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mingcheng/SimpleCompiler/src/front/lexical.cpp > CMakeFiles/Front.dir/src/front/lexical.cpp.i

CMakeFiles/Front.dir/src/front/lexical.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Front.dir/src/front/lexical.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mingcheng/SimpleCompiler/src/front/lexical.cpp -o CMakeFiles/Front.dir/src/front/lexical.cpp.s

CMakeFiles/Front.dir/src/front/semantic.cpp.o: CMakeFiles/Front.dir/flags.make
CMakeFiles/Front.dir/src/front/semantic.cpp.o: ../src/front/semantic.cpp
CMakeFiles/Front.dir/src/front/semantic.cpp.o: CMakeFiles/Front.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mingcheng/SimpleCompiler/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Front.dir/src/front/semantic.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Front.dir/src/front/semantic.cpp.o -MF CMakeFiles/Front.dir/src/front/semantic.cpp.o.d -o CMakeFiles/Front.dir/src/front/semantic.cpp.o -c /home/mingcheng/SimpleCompiler/src/front/semantic.cpp

CMakeFiles/Front.dir/src/front/semantic.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Front.dir/src/front/semantic.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mingcheng/SimpleCompiler/src/front/semantic.cpp > CMakeFiles/Front.dir/src/front/semantic.cpp.i

CMakeFiles/Front.dir/src/front/semantic.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Front.dir/src/front/semantic.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mingcheng/SimpleCompiler/src/front/semantic.cpp -o CMakeFiles/Front.dir/src/front/semantic.cpp.s

CMakeFiles/Front.dir/src/front/syntax.cpp.o: CMakeFiles/Front.dir/flags.make
CMakeFiles/Front.dir/src/front/syntax.cpp.o: ../src/front/syntax.cpp
CMakeFiles/Front.dir/src/front/syntax.cpp.o: CMakeFiles/Front.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mingcheng/SimpleCompiler/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/Front.dir/src/front/syntax.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Front.dir/src/front/syntax.cpp.o -MF CMakeFiles/Front.dir/src/front/syntax.cpp.o.d -o CMakeFiles/Front.dir/src/front/syntax.cpp.o -c /home/mingcheng/SimpleCompiler/src/front/syntax.cpp

CMakeFiles/Front.dir/src/front/syntax.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Front.dir/src/front/syntax.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mingcheng/SimpleCompiler/src/front/syntax.cpp > CMakeFiles/Front.dir/src/front/syntax.cpp.i

CMakeFiles/Front.dir/src/front/syntax.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Front.dir/src/front/syntax.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mingcheng/SimpleCompiler/src/front/syntax.cpp -o CMakeFiles/Front.dir/src/front/syntax.cpp.s

CMakeFiles/Front.dir/src/front/token.cpp.o: CMakeFiles/Front.dir/flags.make
CMakeFiles/Front.dir/src/front/token.cpp.o: ../src/front/token.cpp
CMakeFiles/Front.dir/src/front/token.cpp.o: CMakeFiles/Front.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mingcheng/SimpleCompiler/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/Front.dir/src/front/token.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Front.dir/src/front/token.cpp.o -MF CMakeFiles/Front.dir/src/front/token.cpp.o.d -o CMakeFiles/Front.dir/src/front/token.cpp.o -c /home/mingcheng/SimpleCompiler/src/front/token.cpp

CMakeFiles/Front.dir/src/front/token.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Front.dir/src/front/token.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mingcheng/SimpleCompiler/src/front/token.cpp > CMakeFiles/Front.dir/src/front/token.cpp.i

CMakeFiles/Front.dir/src/front/token.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Front.dir/src/front/token.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mingcheng/SimpleCompiler/src/front/token.cpp -o CMakeFiles/Front.dir/src/front/token.cpp.s

# Object files for target Front
Front_OBJECTS = \
"CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o" \
"CMakeFiles/Front.dir/src/front/lexical.cpp.o" \
"CMakeFiles/Front.dir/src/front/semantic.cpp.o" \
"CMakeFiles/Front.dir/src/front/syntax.cpp.o" \
"CMakeFiles/Front.dir/src/front/token.cpp.o"

# External object files for target Front
Front_EXTERNAL_OBJECTS =

../bin/libFront.a: CMakeFiles/Front.dir/src/front/abstract_syntax_tree.cpp.o
../bin/libFront.a: CMakeFiles/Front.dir/src/front/lexical.cpp.o
../bin/libFront.a: CMakeFiles/Front.dir/src/front/semantic.cpp.o
../bin/libFront.a: CMakeFiles/Front.dir/src/front/syntax.cpp.o
../bin/libFront.a: CMakeFiles/Front.dir/src/front/token.cpp.o
../bin/libFront.a: CMakeFiles/Front.dir/build.make
../bin/libFront.a: CMakeFiles/Front.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mingcheng/SimpleCompiler/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX static library ../bin/libFront.a"
	$(CMAKE_COMMAND) -P CMakeFiles/Front.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Front.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Front.dir/build: ../bin/libFront.a
.PHONY : CMakeFiles/Front.dir/build

CMakeFiles/Front.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Front.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Front.dir/clean

CMakeFiles/Front.dir/depend:
	cd /home/mingcheng/SimpleCompiler/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mingcheng/SimpleCompiler /home/mingcheng/SimpleCompiler /home/mingcheng/SimpleCompiler/build /home/mingcheng/SimpleCompiler/build /home/mingcheng/SimpleCompiler/build/CMakeFiles/Front.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Front.dir/depend

