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
CMAKE_SOURCE_DIR = /home/lrf/Code/TFTP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lrf/Code/TFTP/build

# Include any dependencies generated for this target.
include CMakeFiles/tft.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tft.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tft.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tft.dir/flags.make

CMakeFiles/tft.dir/main.c.o: CMakeFiles/tft.dir/flags.make
CMakeFiles/tft.dir/main.c.o: ../main.c
CMakeFiles/tft.dir/main.c.o: CMakeFiles/tft.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lrf/Code/TFTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/tft.dir/main.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/tft.dir/main.c.o -MF CMakeFiles/tft.dir/main.c.o.d -o CMakeFiles/tft.dir/main.c.o -c /home/lrf/Code/TFTP/main.c

CMakeFiles/tft.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/tft.dir/main.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/lrf/Code/TFTP/main.c > CMakeFiles/tft.dir/main.c.i

CMakeFiles/tft.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/tft.dir/main.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/lrf/Code/TFTP/main.c -o CMakeFiles/tft.dir/main.c.s

CMakeFiles/tft.dir/tftp_base.c.o: CMakeFiles/tft.dir/flags.make
CMakeFiles/tft.dir/tftp_base.c.o: ../tftp_base.c
CMakeFiles/tft.dir/tftp_base.c.o: CMakeFiles/tft.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lrf/Code/TFTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/tft.dir/tftp_base.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/tft.dir/tftp_base.c.o -MF CMakeFiles/tft.dir/tftp_base.c.o.d -o CMakeFiles/tft.dir/tftp_base.c.o -c /home/lrf/Code/TFTP/tftp_base.c

CMakeFiles/tft.dir/tftp_base.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/tft.dir/tftp_base.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/lrf/Code/TFTP/tftp_base.c > CMakeFiles/tft.dir/tftp_base.c.i

CMakeFiles/tft.dir/tftp_base.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/tft.dir/tftp_base.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/lrf/Code/TFTP/tftp_base.c -o CMakeFiles/tft.dir/tftp_base.c.s

CMakeFiles/tft.dir/tftp_client.c.o: CMakeFiles/tft.dir/flags.make
CMakeFiles/tft.dir/tftp_client.c.o: ../tftp_client.c
CMakeFiles/tft.dir/tftp_client.c.o: CMakeFiles/tft.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lrf/Code/TFTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/tft.dir/tftp_client.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/tft.dir/tftp_client.c.o -MF CMakeFiles/tft.dir/tftp_client.c.o.d -o CMakeFiles/tft.dir/tftp_client.c.o -c /home/lrf/Code/TFTP/tftp_client.c

CMakeFiles/tft.dir/tftp_client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/tft.dir/tftp_client.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/lrf/Code/TFTP/tftp_client.c > CMakeFiles/tft.dir/tftp_client.c.i

CMakeFiles/tft.dir/tftp_client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/tft.dir/tftp_client.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/lrf/Code/TFTP/tftp_client.c -o CMakeFiles/tft.dir/tftp_client.c.s

# Object files for target tft
tft_OBJECTS = \
"CMakeFiles/tft.dir/main.c.o" \
"CMakeFiles/tft.dir/tftp_base.c.o" \
"CMakeFiles/tft.dir/tftp_client.c.o"

# External object files for target tft
tft_EXTERNAL_OBJECTS =

tft: CMakeFiles/tft.dir/main.c.o
tft: CMakeFiles/tft.dir/tftp_base.c.o
tft: CMakeFiles/tft.dir/tftp_client.c.o
tft: CMakeFiles/tft.dir/build.make
tft: CMakeFiles/tft.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lrf/Code/TFTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable tft"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tft.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tft.dir/build: tft
.PHONY : CMakeFiles/tft.dir/build

CMakeFiles/tft.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tft.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tft.dir/clean

CMakeFiles/tft.dir/depend:
	cd /home/lrf/Code/TFTP/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lrf/Code/TFTP /home/lrf/Code/TFTP /home/lrf/Code/TFTP/build /home/lrf/Code/TFTP/build /home/lrf/Code/TFTP/build/CMakeFiles/tft.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tft.dir/depend

