# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/mnt/d/share/code/docs/2_TCP 编程基础/srcs"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build"

# Include any dependencies generated for this target.
include CMakeFiles/network_info.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/network_info.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/network_info.dir/flags.make

CMakeFiles/network_info.dir/network_info.c.o: CMakeFiles/network_info.dir/flags.make
CMakeFiles/network_info.dir/network_info.c.o: ../network_info.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/network_info.dir/network_info.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/network_info.dir/network_info.c.o   -c "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/network_info.c"

CMakeFiles/network_info.dir/network_info.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/network_info.dir/network_info.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/network_info.c" > CMakeFiles/network_info.dir/network_info.c.i

CMakeFiles/network_info.dir/network_info.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/network_info.dir/network_info.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/network_info.c" -o CMakeFiles/network_info.dir/network_info.c.s

# Object files for target network_info
network_info_OBJECTS = \
"CMakeFiles/network_info.dir/network_info.c.o"

# External object files for target network_info
network_info_EXTERNAL_OBJECTS =

bin/network_info: CMakeFiles/network_info.dir/network_info.c.o
bin/network_info: CMakeFiles/network_info.dir/build.make
bin/network_info: CMakeFiles/network_info.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable bin/network_info"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/network_info.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/network_info.dir/build: bin/network_info

.PHONY : CMakeFiles/network_info.dir/build

CMakeFiles/network_info.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/network_info.dir/cmake_clean.cmake
.PHONY : CMakeFiles/network_info.dir/clean

CMakeFiles/network_info.dir/depend:
	cd "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/mnt/d/share/code/docs/2_TCP 编程基础/srcs" "/mnt/d/share/code/docs/2_TCP 编程基础/srcs" "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build" "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build" "/mnt/d/share/code/docs/2_TCP 编程基础/srcs/build/CMakeFiles/network_info.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/network_info.dir/depend

