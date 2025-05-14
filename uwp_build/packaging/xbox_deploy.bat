@echo off
REM SRB2P Xbox Deployment Script
REM This batch file automates the process of building and deploying SRB2P to an Xbox

setlocal EnableDelayedExpansion

echo ====== SRB2P Xbox Deployment Tool ======
echo.

REM Check if PowerShell is available
where powershell >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: PowerShell is required but not found.
    exit /b 1
)

REM Set default values
set SOURCE_DIR=%~dp0..\..
set BUILD_DIR=%SOURCE_DIR%\build-uwp
set CONFIG=Release
set DEPLOY=false
set XBOX_IP=

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--source" (
    set SOURCE_DIR=%~2
    shift
) else if /i "%~1"=="--build" (
    set BUILD_DIR=%~2
    shift
) else if /i "%~1"=="--config" (
    set CONFIG=%~2
    shift
) else if /i "%~1"=="--deploy" (
    set DEPLOY=true
) else if /i "%~1"=="--xbox-ip" (
    set XBOX_IP=%~2
    shift
) else (
    echo Unknown option: %~1
    goto :usage
)
shift
goto :parse_args

:usage
echo Usage: xbox_deploy.bat [options]
echo Options:
echo   --source DIR     Source directory (default: parent of script dir)
echo   --build DIR      Build directory (default: build-uwp in source dir)
echo   --config CONFIG  Build configuration (default: Release)
echo   --deploy         Deploy to Xbox after building
echo   --xbox-ip IP     Xbox IP address for direct deployment
exit /b 1

:args_done

REM Ensure source directory exists
if not exist "%SOURCE_DIR%" (
    echo Error: Source directory '%SOURCE_DIR%' does not exist.
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" (
    echo Creating build directory: %BUILD_DIR%
    mkdir "%BUILD_DIR%"
    if %ERRORLEVEL% NEQ 0 (
        echo Error: Failed to create build directory.
        exit /b 1
    )
)

REM Check if Visual Studio is installed (via vswhere)
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set VS_PATH=%%i
)

if "%VS_PATH%"=="" (
    echo Error: Visual Studio not found. Please install Visual Studio with UWP development tools.
    exit /b 1
)

REM Add MSBuild to path
set "PATH=%VS_PATH%\MSBuild\Current\Bin;%PATH%"

REM Check for Windows 10 SDK
if not exist "%ProgramFiles(x86)%\Windows Kits\10" (
    echo Error: Windows 10 SDK not found. Please install the Windows 10 SDK.
    exit /b 1
)

echo Building SRB2P for Xbox...
echo Source directory: %SOURCE_DIR%
echo Build directory: %BUILD_DIR%
echo Configuration: %CONFIG%
echo.

REM Call the PowerShell packaging script
set PS_SCRIPT=%~dp0package.ps1
set PS_ARGS=-SourceDir "%SOURCE_DIR%" -BuildDir "%BUILD_DIR%" -Configuration "%CONFIG%"

if "%DEPLOY%"=="true" (
    set PS_ARGS=%PS_ARGS% -Deploy
)

if not "%XBOX_IP%"=="" (
    set PS_ARGS=%PS_ARGS% -XboxIP "%XBOX_IP%"
)

echo Executing PowerShell packaging script...
powershell -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %PS_ARGS%

if %ERRORLEVEL% NEQ 0 (
    echo Error: Packaging failed.
    exit /b %ERRORLEVEL%
)

echo.
echo SRB2P Xbox build and packaging completed successfully.
echo.

endlocal