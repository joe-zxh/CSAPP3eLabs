# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_COMMAND = /opt/Clion-2017.3.5/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/Clion-2017.3.5/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/joe/ws/cpp/lab7_proxy

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/joe/ws/cpp/lab7_proxy/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/lab7_proxy.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lab7_proxy.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lab7_proxy.dir/flags.make

CMakeFiles/lab7_proxy.dir/proxy.c.o: CMakeFiles/lab7_proxy.dir/flags.make
CMakeFiles/lab7_proxy.dir/proxy.c.o: ../proxy.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/joe/ws/cpp/lab7_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/lab7_proxy.dir/proxy.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lab7_proxy.dir/proxy.c.o   -c /home/joe/ws/cpp/lab7_proxy/proxy.c

CMakeFiles/lab7_proxy.dir/proxy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lab7_proxy.dir/proxy.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/joe/ws/cpp/lab7_proxy/proxy.c > CMakeFiles/lab7_proxy.dir/proxy.c.i

CMakeFiles/lab7_proxy.dir/proxy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lab7_proxy.dir/proxy.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/joe/ws/cpp/lab7_proxy/proxy.c -o CMakeFiles/lab7_proxy.dir/proxy.c.s

CMakeFiles/lab7_proxy.dir/proxy.c.o.requires:

.PHONY : CMakeFiles/lab7_proxy.dir/proxy.c.o.requires

CMakeFiles/lab7_proxy.dir/proxy.c.o.provides: CMakeFiles/lab7_proxy.dir/proxy.c.o.requires
	$(MAKE) -f CMakeFiles/lab7_proxy.dir/build.make CMakeFiles/lab7_proxy.dir/proxy.c.o.provides.build
.PHONY : CMakeFiles/lab7_proxy.dir/proxy.c.o.provides

CMakeFiles/lab7_proxy.dir/proxy.c.o.provides.build: CMakeFiles/lab7_proxy.dir/proxy.c.o


CMakeFiles/lab7_proxy.dir/csapp.c.o: CMakeFiles/lab7_proxy.dir/flags.make
CMakeFiles/lab7_proxy.dir/csapp.c.o: ../csapp.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/joe/ws/cpp/lab7_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/lab7_proxy.dir/csapp.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lab7_proxy.dir/csapp.c.o   -c /home/joe/ws/cpp/lab7_proxy/csapp.c

CMakeFiles/lab7_proxy.dir/csapp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lab7_proxy.dir/csapp.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/joe/ws/cpp/lab7_proxy/csapp.c > CMakeFiles/lab7_proxy.dir/csapp.c.i

CMakeFiles/lab7_proxy.dir/csapp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lab7_proxy.dir/csapp.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/joe/ws/cpp/lab7_proxy/csapp.c -o CMakeFiles/lab7_proxy.dir/csapp.c.s

CMakeFiles/lab7_proxy.dir/csapp.c.o.requires:

.PHONY : CMakeFiles/lab7_proxy.dir/csapp.c.o.requires

CMakeFiles/lab7_proxy.dir/csapp.c.o.provides: CMakeFiles/lab7_proxy.dir/csapp.c.o.requires
	$(MAKE) -f CMakeFiles/lab7_proxy.dir/build.make CMakeFiles/lab7_proxy.dir/csapp.c.o.provides.build
.PHONY : CMakeFiles/lab7_proxy.dir/csapp.c.o.provides

CMakeFiles/lab7_proxy.dir/csapp.c.o.provides.build: CMakeFiles/lab7_proxy.dir/csapp.c.o


# Object files for target lab7_proxy
lab7_proxy_OBJECTS = \
"CMakeFiles/lab7_proxy.dir/proxy.c.o" \
"CMakeFiles/lab7_proxy.dir/csapp.c.o"

# External object files for target lab7_proxy
lab7_proxy_EXTERNAL_OBJECTS =

lab7_proxy: CMakeFiles/lab7_proxy.dir/proxy.c.o
lab7_proxy: CMakeFiles/lab7_proxy.dir/csapp.c.o
lab7_proxy: CMakeFiles/lab7_proxy.dir/build.make
lab7_proxy: CMakeFiles/lab7_proxy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/joe/ws/cpp/lab7_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable lab7_proxy"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lab7_proxy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lab7_proxy.dir/build: lab7_proxy

.PHONY : CMakeFiles/lab7_proxy.dir/build

CMakeFiles/lab7_proxy.dir/requires: CMakeFiles/lab7_proxy.dir/proxy.c.o.requires
CMakeFiles/lab7_proxy.dir/requires: CMakeFiles/lab7_proxy.dir/csapp.c.o.requires

.PHONY : CMakeFiles/lab7_proxy.dir/requires

CMakeFiles/lab7_proxy.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lab7_proxy.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lab7_proxy.dir/clean

CMakeFiles/lab7_proxy.dir/depend:
	cd /home/joe/ws/cpp/lab7_proxy/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/joe/ws/cpp/lab7_proxy /home/joe/ws/cpp/lab7_proxy /home/joe/ws/cpp/lab7_proxy/cmake-build-debug /home/joe/ws/cpp/lab7_proxy/cmake-build-debug /home/joe/ws/cpp/lab7_proxy/cmake-build-debug/CMakeFiles/lab7_proxy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lab7_proxy.dir/depend

