# SRB2P Xbox Packaging Tools

This directory contains scripts and tools to help build and package SRB2P for Xbox Developer Mode.

## Quick Start

### Windows

1. Open a command prompt in this directory
2. Run `xbox_deploy.bat --deploy`
3. Follow the on-screen instructions

## Available Tools

### package.ps1

A PowerShell script that handles the complete build, packaging, and deployment process.

**Usage:**
```powershell
.\package.ps1 [-SourceDir <path>] [-BuildDir <path>] [-Configuration <config>] [-Deploy] [-XboxIP <ip>] [-CertPath <path>] [-CertPass <password>]
```

**Parameters:**
- `-SourceDir`: Path to source directory (default: ".")
- `-BuildDir`: Path to build directory (default: "build-uwp")
- `-Configuration`: Build configuration (default: "Release")
- `-Deploy`: Deploy to Xbox after building
- `-XboxIP`: Xbox IP address for direct deployment
- `-CertPath`: Path to certificate file for signing
- `-CertPass`: Password for certificate file (default: "password")

### xbox_deploy.bat

A batch file wrapper around package.ps1 for easier command-line use.

**Usage:**
```batch
xbox_deploy.bat [--source <path>] [--build <path>] [--config <config>] [--deploy] [--xbox-ip <ip>]
```

**Parameters:**
- `--source`: Path to source directory (default: parent directory of script)
- `--build`: Path to build directory (default: "build-uwp" in source directory)
- `--config`: Build configuration (default: "Release")
- `--deploy`: Deploy to Xbox after building
- `--xbox-ip`: Xbox IP address for direct deployment

## Deployment Methods

### Method 1: Transfer Package

This is the default method and requires the Xbox Device Portal app to be installed on your PC.

1. Build and package the app using `xbox_deploy.bat --deploy`
2. The package will be copied to the Xbox transfer directory
3. Open Dev Home on your Xbox
4. Go to "My games & apps" to find and install SRB2P

### Method 2: Direct Deployment (Coming Soon)

This method will directly deploy to an Xbox on your network using the Windows Device Portal. 
Currently not implemented.

## Troubleshooting

- **Error: Visual Studio not found**: Install Visual Studio with UWP development tools
- **Error: Windows 10 SDK not found**: Install the Windows 10 SDK
- **Error: Transfer directory not found**: Make sure Xbox Device Portal is installed
- **Error: Xbox not found**: Ensure your Xbox is in Developer Mode and on the same network