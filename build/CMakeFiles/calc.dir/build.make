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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.22.3/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.22.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/wukong/Projects/Random/scenario-game

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/wukong/Projects/Random/scenario-game/build

# Include any dependencies generated for this target.
include CMakeFiles/calc.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/calc.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/calc.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/calc.dir/flags.make

CMakeFiles/calc.dir/src/calc.c.o: CMakeFiles/calc.dir/flags.make
CMakeFiles/calc.dir/src/calc.c.o: ../src/calc.c
CMakeFiles/calc.dir/src/calc.c.o: CMakeFiles/calc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/wukong/Projects/Random/scenario-game/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/calc.dir/src/calc.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/calc.dir/src/calc.c.o -MF CMakeFiles/calc.dir/src/calc.c.o.d -o CMakeFiles/calc.dir/src/calc.c.o -c /Users/wukong/Projects/Random/scenario-game/src/calc.c

CMakeFiles/calc.dir/src/calc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/calc.dir/src/calc.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/wukong/Projects/Random/scenario-game/src/calc.c > CMakeFiles/calc.dir/src/calc.c.i

CMakeFiles/calc.dir/src/calc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/calc.dir/src/calc.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/wukong/Projects/Random/scenario-game/src/calc.c -o CMakeFiles/calc.dir/src/calc.c.s

# Object files for target calc
calc_OBJECTS = \
"CMakeFiles/calc.dir/src/calc.c.o"

# External object files for target calc
calc_EXTERNAL_OBJECTS =

libcalc.a: CMakeFiles/calc.dir/src/calc.c.o
libcalc.a: CMakeFiles/calc.dir/build.make
libcalc.a: CMakeFiles/calc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/wukong/Projects/Random/scenario-game/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libcalc.a"
	$(CMAKE_COMMAND) -P CMakeFiles/calc.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/calc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/calc.dir/build: libcalc.a
.PHONY : CMakeFiles/calc.dir/build

CMakeFiles/calc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/calc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/calc.dir/clean

CMakeFiles/calc.dir/depend:
	cd /Users/wukong/Projects/Random/scenario-game/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/wukong/Projects/Random/scenario-game /Users/wukong/Projects/Random/scenario-game /Users/wukong/Projects/Random/scenario-game/build /Users/wukong/Projects/Random/scenario-game/build /Users/wukong/Projects/Random/scenario-game/build/CMakeFiles/calc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/calc.dir/depend

