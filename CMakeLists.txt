# The first line of any CMake project should be a call to `cmake_minimum_required`, which checks
# that the installed CMake will be able to understand the following CMakeLists, and ensures that
# CMake's behaviour is compatible with the named version. This is a standard CMake command, so more
# information can be found in the CMake docs.

cmake_minimum_required(VERSION 3.24)

# Sets the minimum macOS version
if (APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum version of the target platform" FORCE) 
	if(CMAKE_OSX_DEPLOYMENT_TARGET)
		message("The minimum macOS version is set to " $CACHE{CMAKE_OSX_DEPLOYMENT_TARGET}.)
	endif()
endif ()

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(PluginTest VERSION 0.0.1)
set (TARGET_NAME ${PROJECT_NAME})

if(APPLE)
	set (FORMATS_TO_BUILD  VST3)
else()
	set (FORMATS_TO_BUILD VST3 )
endif()


add_subdirectory(modules/JUCE)                    # If you've put JUCE in a subdirectory called JUCE


juce_add_plugin(${TARGET_NAME}
		# VERSION ...                               # Set this if the plugin version is different to the project version
		# ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
		# ICON_SMALL ...
		COMPANY_NAME "RAISE"
		# IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
		# NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
		# NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
		# IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
		# EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
		# COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
		PLUGIN_MANUFACTURER_CODE RAIS               # A four-character manufacturer id with at least one upper-case character
		PLUGIN_CODE RAIS                            # A unique four-character plugin id with exactly one upper-case character
		# GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case

		if(APPLE)
			HARDENED_RUNTIME_ENABLED TRUE
			HARDENED_RUNTIME_OPTIONS "com.apple.security.device.audio-input"
			MICROPHONE_PERMISSION_ENABLED TRUE
			MICROPHONE_PERMISSION_TEXT "Need access to your audio interface"
		endif()

		FORMATS ${FORMATS_TO_BUILD}                    # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
		PRODUCT_NAME "PluginTest"			            # The name of the final executable, which can differ from the target name
		)

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

juce_generate_juce_header(${TARGET_NAME})

# Sets the cpp language minimum
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Add all source files to file list
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)
# Include the main directory and its subdirectories
include_directories(${CMAKE_SOURCE_DIR}/libs)

# Add all sources to target
target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

# Add include directories for all folders in the source
file(GLOB_RECURSE source_dirs LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/source/*)

foreach (dir ${source_dirs})
    if (IS_DIRECTORY ${dir})
        target_include_directories(${TARGET_NAME} PRIVATE ${dir})
    endif ()
endforeach ()

# Make the folder structure visible in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "source" FILES ${SOURCES})

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(${TARGET_NAME}
		PUBLIC
		# JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
		JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
		JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
		JUCE_VST3_CAN_REPLACE_VST2=0
		JUCE_DISPLAY_SPLASH_SCREEN=1
		DONT_SET_USING_JUCE_NAMESPACE=1
		)



target_link_libraries(${TARGET_NAME}
		PRIVATE
		juce::juce_audio_utils
		juce::juce_dsp
		juce::juce_opengl
		juce::juce_graphics
		juce::juce_gui_basics
		juce::juce_gui_extra
		

		PUBLIC
		juce::juce_recommended_config_flags
		juce::juce_recommended_lto_flags
		juce::juce_recommended_warning_flags
)