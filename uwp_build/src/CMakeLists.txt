# UWP-specific source files
set(SRB2_UWP_SOURCES
    uwp_main.cpp
)

# Add UWP resource files
set(SRB2_UWP_RESOURCES
    ../assets/uwp_resources.rc
)

# Get the original SRB2 sources from Sourcefile
file(STRINGS ../../Sourcefile SRB2_SOURCES_RAW)

# Create filtered lists for sources
set(SRB2_C_SOURCES "")
set(SRB2_CPP_SOURCES "")
set(SRB2_HEADERS "")
foreach(SOURCE ${SRB2_SOURCES_RAW})
    # Skip the SDL/Windows main.c that we're replacing with our UWP version
    if(NOT SOURCE MATCHES ".*sdl/i_main.c$" AND NOT SOURCE MATCHES ".*win32/win_main.c$")
        if(SOURCE MATCHES ".*\\.c$")
            list(APPEND SRB2_C_SOURCES "../../${SOURCE}")
        elseif(SOURCE MATCHES ".*\\.cpp$")
            list(APPEND SRB2_CPP_SOURCES "../../${SOURCE}")
        elseif(SOURCE MATCHES ".*\\.h$")
            list(APPEND SRB2_HEADERS "../../${SOURCE}")
        endif()
    endif()
endforeach()

# Create the application package for Xbox/UWP
add_executable(SRB2SDL2_UWP WIN32 
    ${SRB2_UWP_SOURCES}
    ${SRB2_C_SOURCES}
    ${SRB2_CPP_SOURCES}
    ${SRB2_HEADERS}
    ${SRB2_UWP_RESOURCES}
)

# Set target properties specific to UWP
set_target_properties(SRB2SDL2_UWP PROPERTIES
    OUTPUT_NAME "SRB2SDL2"
    VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.16299.0"
    VS_WINDOWS_APP_TYPE "UWP"
    VS_GLOBAL_AppxPackageDir "${CMAKE_BINARY_DIR}/AppPackages/"
    VS_GLOBAL_AppxBundle "Always"
    VS_GLOBAL_AppxBundlePlatforms "x64"
)

# Include directories
target_include_directories(SRB2SDL2_UWP PRIVATE
    ../../src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# UWP-specific definitions
target_compile_definitions(SRB2SDL2_UWP PRIVATE
    _UNICODE
    UNICODE
    WINRT
    WINDOWS_UWP
    SRB2_UWP
    SRB2_XBOX
)

# UWP-specific compiler flags
target_compile_options(SRB2SDL2_UWP PRIVATE
    /ZW
    /EHsc
    /await
)

# Link UWP-specific libraries
target_link_libraries(SRB2SDL2_UWP PRIVATE
    # Standard UWP libraries
    WindowsApp.lib
    
    # SDL libraries adapted for UWP
    SDL2::SDL2-static
    SDL2_mixer::SDL2_mixer-static
    
    # Other dependencies
    ZLIB::ZLIB
    PNG::PNG
    CURL::libcurl
)

# Add resources
target_sources(SRB2SDL2_UWP PRIVATE
    Assets/SplashScreen.png
    Assets/Square150x150Logo.png
    Assets/Square44x44Logo.png
    Assets/StoreLogo.png
    Assets/Wide310x150Logo.png
)

# Add AppxManifest
set_property(SOURCE ${CMAKE_BINARY_DIR}/AppxManifest.xml PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${CMAKE_BINARY_DIR}/AppxManifest.xml PROPERTY VS_DEPLOYMENT_LOCATION "")

# Add Xbox UWP capabilities
add_custom_command(TARGET SRB2SDL2_UWP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_CURRENT_SOURCE_DIR}/../AppxManifest.xml.in 
        ${CMAKE_BINARY_DIR}/AppxManifest.xml
    COMMENT "Copying AppxManifest.xml for Xbox UWP"
)