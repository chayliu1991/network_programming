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
CMAKE_SOURCE_DIR = /mnt/d/share/code

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/share/code/build

# Include any dependencies generated for this target.
include CMakeFiles/grace_client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/grace_client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/grace_client.dir/flags.make

CMakeFiles/grace_client.dir/grace_client.c.o: CMakeFiles/grace_client.dir/flags.make
CMakeFiles/grace_client.dir/grace_client.c.o: ../grace_client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/share/code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/grace_client.dir/grace_client.c.o"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/grace_client.dir/grace_client.c.o   -c /mnt/d/share/code/grace_client.c

CMakeFiles/grace_client.dir/grace_client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/grace_client.dir/grace_client.c.i"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/d/share/code/grace_client.c > CMakeFiles/grace_client.dir/grace_client.c.i

CMakeFiles/grace_client.dir/grace_client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/grace_client.dir/grace_client.c.s"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/d/share/code/grace_client.c -o CMakeFiles/grace_client.dir/grace_client.c.s

# Object files for target grace_client
grace_client_OBJECTS = \
"CMakeFiles/grace_client.dir/grace_client.c.o"

# External object files for target grace_client
grace_client_EXTERNAL_OBJECTS =

grace_client: CMakeFiles/grace_client.dir/grace_client.c.o
grace_client: CMakeFiles/grace_client.dir/build.make
grace_client: CMakeFiles/grace_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/d/share/code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable grace_client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/grace_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/grace_client.dir/build: grace_client

.PHONY : CMakeFiles/grace_client.dir/build

CMakeFiles/grace_client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/grace_client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/grace_client.dir/clean

CMakeFiles/grace_client.dir/depend:
	cd /mnt/d/share/code/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/d/share/code /mnt/d/share/code /mnt/d/share/code/build /mnt/d/share/code/build /mnt/d/share/code/build/CMakeFiles/grace_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/grace_client.dir/depend

