# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/tim/DIGITAL-INPUT-OUTPUT/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tim/DIGITAL-INPUT-OUTPUT/build

# Include any dependencies generated for this target.
include porter_odom/CMakeFiles/porter_odom.dir/depend.make

# Include the progress variables for this target.
include porter_odom/CMakeFiles/porter_odom.dir/progress.make

# Include the compile flags for this target's objects.
include porter_odom/CMakeFiles/porter_odom.dir/flags.make

porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o: porter_odom/CMakeFiles/porter_odom.dir/flags.make
porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o: /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/encoder_lib.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tim/DIGITAL-INPUT-OUTPUT/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o -c /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/encoder_lib.cpp

porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.i"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/encoder_lib.cpp > CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.i

porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.s"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/encoder_lib.cpp -o CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.s

porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.requires:

.PHONY : porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.requires

porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.provides: porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.requires
	$(MAKE) -f porter_odom/CMakeFiles/porter_odom.dir/build.make porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.provides.build
.PHONY : porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.provides

porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.provides.build: porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o


porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o: porter_odom/CMakeFiles/porter_odom.dir/flags.make
porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o: /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/porter_odom.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tim/DIGITAL-INPUT-OUTPUT/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o -c /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/porter_odom.cpp

porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/porter_odom.dir/src/porter_odom.cpp.i"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/porter_odom.cpp > CMakeFiles/porter_odom.dir/src/porter_odom.cpp.i

porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/porter_odom.dir/src/porter_odom.cpp.s"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom/src/porter_odom.cpp -o CMakeFiles/porter_odom.dir/src/porter_odom.cpp.s

porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.requires:

.PHONY : porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.requires

porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.provides: porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.requires
	$(MAKE) -f porter_odom/CMakeFiles/porter_odom.dir/build.make porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.provides.build
.PHONY : porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.provides

porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.provides.build: porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o


# Object files for target porter_odom
porter_odom_OBJECTS = \
"CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o" \
"CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o"

# External object files for target porter_odom
porter_odom_EXTERNAL_OBJECTS =

/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: porter_odom/CMakeFiles/porter_odom.dir/build.make
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libtf.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libtf2_ros.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libactionlib.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libmessage_filters.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libroscpp.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_signals.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libxmlrpcpp.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libtf2.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libroscpp_serialization.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/librosconsole.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/librosconsole_log4cxx.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/librosconsole_backend_interface.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/liblog4cxx.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_regex.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/librostime.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /opt/ros/melodic/lib/libcpp_common.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_system.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_thread.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_chrono.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libboost_atomic.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libpthread.so
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: /usr/lib/x86_64-linux-gnu/libconsole_bridge.so.0.4
/home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom: porter_odom/CMakeFiles/porter_odom.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tim/DIGITAL-INPUT-OUTPUT/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable /home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom"
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/porter_odom.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
porter_odom/CMakeFiles/porter_odom.dir/build: /home/tim/DIGITAL-INPUT-OUTPUT/devel/lib/porter_odom/porter_odom

.PHONY : porter_odom/CMakeFiles/porter_odom.dir/build

porter_odom/CMakeFiles/porter_odom.dir/requires: porter_odom/CMakeFiles/porter_odom.dir/src/encoder_lib.cpp.o.requires
porter_odom/CMakeFiles/porter_odom.dir/requires: porter_odom/CMakeFiles/porter_odom.dir/src/porter_odom.cpp.o.requires

.PHONY : porter_odom/CMakeFiles/porter_odom.dir/requires

porter_odom/CMakeFiles/porter_odom.dir/clean:
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom && $(CMAKE_COMMAND) -P CMakeFiles/porter_odom.dir/cmake_clean.cmake
.PHONY : porter_odom/CMakeFiles/porter_odom.dir/clean

porter_odom/CMakeFiles/porter_odom.dir/depend:
	cd /home/tim/DIGITAL-INPUT-OUTPUT/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tim/DIGITAL-INPUT-OUTPUT/src /home/tim/DIGITAL-INPUT-OUTPUT/src/porter_odom /home/tim/DIGITAL-INPUT-OUTPUT/build /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom /home/tim/DIGITAL-INPUT-OUTPUT/build/porter_odom/CMakeFiles/porter_odom.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : porter_odom/CMakeFiles/porter_odom.dir/depend

