# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

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
CMAKE_SOURCE_DIR = /home/pi/gpu_main/raspicam

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/gpu_main/raspicam/build

# Include any dependencies generated for this target.
include CMakeFiles/raspivid.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/raspivid.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/raspivid.dir/flags.make

CMakeFiles/raspivid.dir/RaspiCamControl.o: CMakeFiles/raspivid.dir/flags.make
CMakeFiles/raspivid.dir/RaspiCamControl.o: ../RaspiCamControl.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/gpu_main/raspicam/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/raspivid.dir/RaspiCamControl.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/raspivid.dir/RaspiCamControl.o   -c /home/pi/gpu_main/raspicam/RaspiCamControl.c

CMakeFiles/raspivid.dir/RaspiCamControl.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/raspivid.dir/RaspiCamControl.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/pi/gpu_main/raspicam/RaspiCamControl.c > CMakeFiles/raspivid.dir/RaspiCamControl.i

CMakeFiles/raspivid.dir/RaspiCamControl.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/raspivid.dir/RaspiCamControl.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/pi/gpu_main/raspicam/RaspiCamControl.c -o CMakeFiles/raspivid.dir/RaspiCamControl.s

CMakeFiles/raspivid.dir/RaspiCamControl.o.requires:
.PHONY : CMakeFiles/raspivid.dir/RaspiCamControl.o.requires

CMakeFiles/raspivid.dir/RaspiCamControl.o.provides: CMakeFiles/raspivid.dir/RaspiCamControl.o.requires
	$(MAKE) -f CMakeFiles/raspivid.dir/build.make CMakeFiles/raspivid.dir/RaspiCamControl.o.provides.build
.PHONY : CMakeFiles/raspivid.dir/RaspiCamControl.o.provides

CMakeFiles/raspivid.dir/RaspiCamControl.o.provides.build: CMakeFiles/raspivid.dir/RaspiCamControl.o

CMakeFiles/raspivid.dir/RaspiCLI.o: CMakeFiles/raspivid.dir/flags.make
CMakeFiles/raspivid.dir/RaspiCLI.o: ../RaspiCLI.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/gpu_main/raspicam/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/raspivid.dir/RaspiCLI.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/raspivid.dir/RaspiCLI.o   -c /home/pi/gpu_main/raspicam/RaspiCLI.c

CMakeFiles/raspivid.dir/RaspiCLI.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/raspivid.dir/RaspiCLI.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/pi/gpu_main/raspicam/RaspiCLI.c > CMakeFiles/raspivid.dir/RaspiCLI.i

CMakeFiles/raspivid.dir/RaspiCLI.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/raspivid.dir/RaspiCLI.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/pi/gpu_main/raspicam/RaspiCLI.c -o CMakeFiles/raspivid.dir/RaspiCLI.s

CMakeFiles/raspivid.dir/RaspiCLI.o.requires:
.PHONY : CMakeFiles/raspivid.dir/RaspiCLI.o.requires

CMakeFiles/raspivid.dir/RaspiCLI.o.provides: CMakeFiles/raspivid.dir/RaspiCLI.o.requires
	$(MAKE) -f CMakeFiles/raspivid.dir/build.make CMakeFiles/raspivid.dir/RaspiCLI.o.provides.build
.PHONY : CMakeFiles/raspivid.dir/RaspiCLI.o.provides

CMakeFiles/raspivid.dir/RaspiCLI.o.provides.build: CMakeFiles/raspivid.dir/RaspiCLI.o

CMakeFiles/raspivid.dir/RaspiPreview.o: CMakeFiles/raspivid.dir/flags.make
CMakeFiles/raspivid.dir/RaspiPreview.o: ../RaspiPreview.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/gpu_main/raspicam/build/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/raspivid.dir/RaspiPreview.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/raspivid.dir/RaspiPreview.o   -c /home/pi/gpu_main/raspicam/RaspiPreview.c

CMakeFiles/raspivid.dir/RaspiPreview.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/raspivid.dir/RaspiPreview.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/pi/gpu_main/raspicam/RaspiPreview.c > CMakeFiles/raspivid.dir/RaspiPreview.i

CMakeFiles/raspivid.dir/RaspiPreview.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/raspivid.dir/RaspiPreview.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/pi/gpu_main/raspicam/RaspiPreview.c -o CMakeFiles/raspivid.dir/RaspiPreview.s

CMakeFiles/raspivid.dir/RaspiPreview.o.requires:
.PHONY : CMakeFiles/raspivid.dir/RaspiPreview.o.requires

CMakeFiles/raspivid.dir/RaspiPreview.o.provides: CMakeFiles/raspivid.dir/RaspiPreview.o.requires
	$(MAKE) -f CMakeFiles/raspivid.dir/build.make CMakeFiles/raspivid.dir/RaspiPreview.o.provides.build
.PHONY : CMakeFiles/raspivid.dir/RaspiPreview.o.provides

CMakeFiles/raspivid.dir/RaspiPreview.o.provides.build: CMakeFiles/raspivid.dir/RaspiPreview.o

CMakeFiles/raspivid.dir/RaspiVid.o: CMakeFiles/raspivid.dir/flags.make
CMakeFiles/raspivid.dir/RaspiVid.o: ../RaspiVid.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/gpu_main/raspicam/build/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/raspivid.dir/RaspiVid.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/raspivid.dir/RaspiVid.o   -c /home/pi/gpu_main/raspicam/RaspiVid.c

CMakeFiles/raspivid.dir/RaspiVid.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/raspivid.dir/RaspiVid.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/pi/gpu_main/raspicam/RaspiVid.c > CMakeFiles/raspivid.dir/RaspiVid.i

CMakeFiles/raspivid.dir/RaspiVid.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/raspivid.dir/RaspiVid.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/pi/gpu_main/raspicam/RaspiVid.c -o CMakeFiles/raspivid.dir/RaspiVid.s

CMakeFiles/raspivid.dir/RaspiVid.o.requires:
.PHONY : CMakeFiles/raspivid.dir/RaspiVid.o.requires

CMakeFiles/raspivid.dir/RaspiVid.o.provides: CMakeFiles/raspivid.dir/RaspiVid.o.requires
	$(MAKE) -f CMakeFiles/raspivid.dir/build.make CMakeFiles/raspivid.dir/RaspiVid.o.provides.build
.PHONY : CMakeFiles/raspivid.dir/RaspiVid.o.provides

CMakeFiles/raspivid.dir/RaspiVid.o.provides.build: CMakeFiles/raspivid.dir/RaspiVid.o

# Object files for target raspivid
raspivid_OBJECTS = \
"CMakeFiles/raspivid.dir/RaspiCamControl.o" \
"CMakeFiles/raspivid.dir/RaspiCLI.o" \
"CMakeFiles/raspivid.dir/RaspiPreview.o" \
"CMakeFiles/raspivid.dir/RaspiVid.o"

# External object files for target raspivid
raspivid_EXTERNAL_OBJECTS =

raspivid: CMakeFiles/raspivid.dir/RaspiCamControl.o
raspivid: CMakeFiles/raspivid.dir/RaspiCLI.o
raspivid: CMakeFiles/raspivid.dir/RaspiPreview.o
raspivid: CMakeFiles/raspivid.dir/RaspiVid.o
raspivid: CMakeFiles/raspivid.dir/build.make
raspivid: CMakeFiles/raspivid.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable raspivid"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/raspivid.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/raspivid.dir/build: raspivid
.PHONY : CMakeFiles/raspivid.dir/build

CMakeFiles/raspivid.dir/requires: CMakeFiles/raspivid.dir/RaspiCamControl.o.requires
CMakeFiles/raspivid.dir/requires: CMakeFiles/raspivid.dir/RaspiCLI.o.requires
CMakeFiles/raspivid.dir/requires: CMakeFiles/raspivid.dir/RaspiPreview.o.requires
CMakeFiles/raspivid.dir/requires: CMakeFiles/raspivid.dir/RaspiVid.o.requires
.PHONY : CMakeFiles/raspivid.dir/requires

CMakeFiles/raspivid.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/raspivid.dir/cmake_clean.cmake
.PHONY : CMakeFiles/raspivid.dir/clean

CMakeFiles/raspivid.dir/depend:
	cd /home/pi/gpu_main/raspicam/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/gpu_main/raspicam /home/pi/gpu_main/raspicam /home/pi/gpu_main/raspicam/build /home/pi/gpu_main/raspicam/build /home/pi/gpu_main/raspicam/build/CMakeFiles/raspivid.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/raspivid.dir/depend

