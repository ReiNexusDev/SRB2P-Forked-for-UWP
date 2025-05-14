cmake_minimum_required(VERSION 3.13)

# Project name
project(SRB2P-UWP)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Unicode builds for UWP
add_definitions(-DUNICODE -D_UNICODE)

# Set some options for SRB2
option(SRB2_CONFIG_HAVE_MIXER "Enable sound" ON)
option(SRB2_CONFIG_HAVE_PNG "Enable PNG support" ON)
option(SRB2_CONFIG_HAVE_CURL "Enable curl support" ON)
option(SRB2_CONFIG_HWRENDER "Enable hardware rendering" ON)
option(SRB2_CONFIG_VANILLA "Disable all optional features" OFF)
option(SRB2_CONFIG_SDL2 "Compile with SDL2" ON)
option(SRB2_CONFIG_WINDOWED "Default to windowed mode" ON)
option(SRB2_UWP "UWP build for Xbox" ON)

# Add UWP-specific source files
add_subdirectory(src)

# Copy AppxManifest.xml template to build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/AppxManifest.xml.in"
    "${CMAKE_BINARY_DIR}/AppxManifest.xml"
    COPYONLY
)

# Copy asset files for UWP packaging
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_BINARY_DIR}/Assets/)

# Package app for Xbox - This creates the APPX bundle
set(APPX_PACKAGE_DIR "${CMAKE_BINARY_DIR}/AppPackages")
set(APPX_BUNDLE_NAME "SRB2P")

# Create a custom target to build the APPX package
add_custom_target(
    create_appx
    COMMAND ${CMAKE_COMMAND} -E make_directory ${APPX_PACKAGE_DIR}
    COMMAND makeappx pack /d "${CMAKE_BINARY_DIR}" /p "${APPX_PACKAGE_DIR}/${APPX_BUNDLE_NAME}.appx" /o
    DEPENDS SRB2SDL2_UWP
    COMMENT "Creating APPX package for Xbox"
)

# Add a custom target to create self-signed certificate and sign the package
add_custom_target(
    sign_appx
    COMMAND powershell -Command "if (!(Test-Path '${CMAKE_BINARY_DIR}/cert.pfx')) { New-SelfSignedCertificate -Type Custom -Subject 'CN=Developer' -KeyUsage DigitalSignature -FriendlyName 'SRB2P Dev Cert' -CertStoreLocation 'Cert:\\CurrentUser\\My' | Export-PfxCertificate -FilePath '${CMAKE_BINARY_DIR}/cert.pfx' -Password (ConvertTo-SecureString -String 'password' -Force -AsPlainText) }"
    COMMAND signtool sign /fd SHA256 /a /f "${CMAKE_BINARY_DIR}/cert.pfx" /p password "${APPX_PACKAGE_DIR}/${APPX_BUNDLE_NAME}.appx"
    DEPENDS create_appx
    COMMENT "Signing APPX package for Xbox"
)

# Target for complete build and packaging process
add_custom_target(
    package_for_xbox ALL
    DEPENDS sign_appx
    COMMENT "Building complete Xbox package"
)

# Print success message when done
add_custom_command(
    TARGET package_for_xbox
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Xbox package built successfully at ${APPX_PACKAGE_DIR}/${APPX_BUNDLE_NAME}.appx"
    COMMENT "Xbox build completed"
)

# Install the package to the Xbox
add_custom_target(
    deploy_to_xbox
    COMMAND ${CMAKE_COMMAND} -E echo "Deploying to Xbox..."
    COMMAND xcopy /y "${APPX_PACKAGE_DIR}\\${APPX_BUNDLE_NAME}.appx" "%USERPROFILE%\\AppData\\Local\\Packages\\Microsoft.XboxDevices_8wekyb3d8bbwe\\LocalState\\DevelopmentFiles\\TransferPackage\\"
    DEPENDS package_for_xbox
    COMMENT "Deploying to Xbox. Make sure DevHome is running on your Xbox!"
)

# Print help message
message(STATUS "=== SRB2P UWP Build for Xbox ===")
message(STATUS "Build targets:")
message(STATUS "  - SRB2SDL2_UWP: Builds the game")
message(STATUS "  - create_appx: Creates APPX package")
message(STATUS "  - sign_appx: Signs the APPX package")
message(STATUS "  - package_for_xbox: Full build + package")
message(STATUS "  - deploy_to_xbox: Deploy to connected Xbox")