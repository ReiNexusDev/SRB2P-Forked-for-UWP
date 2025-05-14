# SRB2P for Xbox - Build Instructions

This document provides detailed instructions for building SRB2P for Xbox Developer Mode.

## Prerequisites

1. **Windows 10/11** with the following software installed:
   - Visual Studio 2019 or 2022 with UWP development tools
   - Windows 10 SDK (10.0.16299.0 or later)
   - CMake 3.13 or later
   - Git

2. **Xbox Developer Mode** enabled on your Xbox console. See [Microsoft's instructions](https://docs.microsoft.com/en-us/windows/uwp/xbox-apps/devkit-activation).

3. **Original SRB2P Source Code** - You must have a local copy of the SRB2P codebase.

## Building SRB2P for Xbox

### Step 1: Prepare the Build Environment

1. Clone or download this repository.
2. Place the UWP build files in the root directory of your SRB2P source code.
3. Create the necessary asset directories:
   ```
   mkdir -p uwp_build/assets/Assets
   ```

4. Copy the required assets to the Assets folder:
   ```
   copy icons/Square150x150Logo.png uwp_build/assets/Assets/
   copy icons/Square44x44Logo.png uwp_build/assets/Assets/
   copy icons/StoreLogo.png uwp_build/assets/Assets/
   copy icons/Wide310x150Logo.png uwp_build/assets/Assets/
   copy icons/SplashScreen.png uwp_build/assets/Assets/
   ```

### Step 2: Configure and Build

1. Open a Developer Command Prompt for Visual Studio.

2. Navigate to the SRB2P source root directory:
   ```
   cd path/to/srb2p
   ```

3. Create a build directory and navigate to it:
   ```
   mkdir build-uwp
   cd build-uwp
   ```

4. Run CMake to configure the project:
   ```
   cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 ../uwp_build
   ```
   Note: If using Visual Studio 2022, use `-G "Visual Studio 17 2022"` instead.

5. Build the solution:
   ```
   cmake --build . --config Release
   ```

6. Create the APPX package:
   ```
   cmake --build . --target package_for_xbox
   ```

### Step 3: Deploy to Xbox

#### Method 1: Using Visual Studio

1. Open the generated solution file in Visual Studio.
2. Right-click on the `SRB2SDL2_UWP` project.
3. Select "Deploy" from the context menu.
4. Make sure your Xbox is connected to the same network and in Developer Mode.

#### Method 2: Using the command line

1. Build the deployment target:
   ```
   cmake --build . --target deploy_to_xbox
   ```

2. On your Xbox, open the Dev Home app.
3. Navigate to the "My games & apps" section.
4. SRB2P should appear in the list after deployment.

## Game Features

The Xbox port of SRB2P includes the following key features:

1. **Full controller support** with native Xbox controller integration
2. **External storage access** through the Xbox file manager
3. **Multiplayer support** for up to 16 players
4. **BLUA/LUA implementation** for mods and custom content

## Troubleshooting

### Common Issues

1. **Build fails with "SDK version not found"**
   - Make sure you have the correct Windows 10 SDK version installed (10.0.16299.0 or later)
   - Try using a different SDK version by changing `CMAKE_SYSTEM_VERSION`

2. **Deployment fails**
   - Make sure your Xbox is in Developer Mode
   - Check that the Xbox and your PC are on the same network
   - Verify that the Developer Mode settings are correctly configured

3. **Game crashes on startup**
   - Check the application logs in Dev Home on your Xbox
   - Make sure all required assets and resources are correctly packaged

### Getting Help

If you encounter issues not covered here, please open an issue on the GitHub repository or contact the development team.