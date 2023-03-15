cmake_minimum_required(VERSION 3.15)

project(bender VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-D_GLIBCXX_DEBUG)
endif()

# global hack to make valgrind work
if(MSVC)
else()
add_compile_options(-gdwarf-4)
endif()

include(FetchContent)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen
  GIT_TAG        3.4.0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_GetProperties(eigen)
if(NOT eigen_POPULATED)
  FetchContent_Populate(eigen)
endif()

find_package(JUCE CONFIG REQUIRED)

if (WIN32)
   set(PLUGIN_FORMATS VST3)
elseif(APPLE)
    set(PLUGIN_FORMATS VST3 AUv3)
elseif(UNIX)
    set(PLUGIN_FORMATS VST3)
else()
    message(FATAL_ERROR "Not WIN32 APPLE or UNIX, who are you?")
endif()

set (MAIN_SOURCES
    src/dsp/gru.cpp
)

set (PLUGIN_SOURCES
    src/processor.cpp
    src/processor_base.cpp
    src/gui/editor.cpp
)


set (COMMON_JUCE_PARAMETERS
    COMPANY_NAME 8ave
    FORMATS ${PLUGIN_FORMATS}
    VST3_CATEGORIES Filter
    AU_MAIN_TYPE kAudioUnitType_Effect
    PLUGIN_MANUFACTURER_CODE 8AVE
    IS_SYNTH 0
    NEEDS_MIDI_INPUT 1
)


add_library(juce_common OBJECT)
target_compile_definitions(juce_common
    PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE=1
    _USE_MATH_DEFINES
    INTERFACE
    $<TARGET_PROPERTY:juce_common,COMPILE_DEFINITIONS>
)

set (COMMON_PUBLIC_TARGET_LINKS
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_link_libraries( juce_common
    PRIVATE
    juce::juce_gui_basics
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_opengl
    juce::juce_video
    PUBLIC
    ${COMMON_PUBLIC_TARGET_LINKS}
)

target_include_directories(juce_common
    INTERFACE
        $<TARGET_PROPERTY:juce_common,INCLUDE_DIRECTORIES>)

set_target_properties(juce_common PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    VISIBILITY_INLINES_HIDDEN TRUE
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden)

add_library(bender_common OBJECT ${MAIN_SOURCES})
target_link_libraries(bender_common PUBLIC juce_common)
set_target_properties(bender_common PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    VISIBILITY_INLINES_HIDDEN TRUE
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden)
target_include_directories(bender_common PUBLIC src ${eigen_SOURCE_DIR})
target_link_libraries(bender_common
PUBLIC
    UIData
    juce_common
)


include(uires.cmake)
include (utilities.cmake)

function( base_plugin target_type target name code )
    message ("Preparing target: ${target} with name ${name} and code ${code} ")

    juce_add_plugin(${target}
        IS_SYNTH 1
        ${COMMON_JUCE_PARAMETERS}
        PLUGIN_NAME ${name}
        PLUGIN_CODE ${code}
        PRODUCT_NAME ${name}) # executable name

    target_sources(${target} PRIVATE ${PLUGIN_SOURCES})

    target_compile_definitions(${target}
        PRIVATE
            PLUGIN_NAME="$<TARGET_PROPERTY:${target},JUCE_PLUGIN_NAME>"
            ${name}
            PLUGIN_CODE=${code}
            DONT_SET_USING_JUCE_NAMESPACE
            JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${target},JUCE_PRODUCT_NAME>"
            JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${target},JUCE_VERSION>"
    )

    get_target_property(juce_library_code ${target} JUCE_GENERATED_SOURCES_DIRECTORY)
    target_include_directories(${target} SYSTEM PRIVATE ${juce_library_code})

    target_link_libraries(${target}
        PRIVATE
            bender_common
            juce_common
    )

    if(MSVC)
    else()
    target_compile_options(${target} PRIVATE -Wno-cast-qual)
    endif()
    if(MSVC)
        set_target_properties(${target} PROPERTIES VS_DEBUGGER_COMMAND "C:\\Program Files\\REAPER %28x64%29\\reaper.exe")
    elseif(APPLE)
        set_target_properties(${target} PROPERTIES XCODE_SCHEME_EXECUTABLE "/Applications/REAPER.app")
    endif()

    add_custom_target( ${target}_print_target_dir ALL
      ${CMAKE_COMMAND} -E echo
      "target dir = $<TARGET_FILE_DIR:${target}_VST3>")

    # copy_all_presets(${target} $<TARGET_FILE_DIR:${target}_VST3>)

endfunction()

base_plugin( plugin bender Bender BNDR )
