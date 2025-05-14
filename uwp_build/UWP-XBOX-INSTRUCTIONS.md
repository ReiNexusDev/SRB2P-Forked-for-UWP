# SRB2P Xbox UWP Port - Deployment Instructions

This guide outlines how to build and deploy the SRB2P (Sonic Robo Blast 2 Persona) port to Xbox Developer Mode using UWP (Universal Windows Platform).

## Prerequisites

1. Windows 10/11 computer
2. Node.js and npm installed
3. [Electron](https://www.electronjs.org/) and [electron-builder](https://www.electron.build/) (installed via npm)
4. Visual Studio with UWP development workload installed
5. Windows 10 SDK (10.0.19041.0 or newer)
6. Xbox Developer Mode activated on your Xbox
7. [Xbox Device Portal](https://learn.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-xbox) access

## Setup Process

### 1. Creating a Certificate

To sign your UWP package, you'll need a certificate. For development purposes, use the provided script:

```powershell
# Run the certificate creation script (PowerShell as Administrator)
.\create-cert.ps1
```

This creates a self-signed certificate for development. For real deployment, you should obtain a proper certificate.

### 2. Building the UWP Package

```powershell
# Run the build script
.\build-uwp.ps1
```

This script:
- Installs all dependencies
- Builds the application
- Packages it as a UWP/APPX bundle suitable for Xbox
- The resulting package will be in the `release-builds` folder

## Deploying to Xbox Developer Mode

### Method 1: Using Xbox Device Portal

1. Put your Xbox in Developer Mode
2. Connect to your Xbox's Device Portal web interface (typically http://[xbox-ip]:11443)
3. Navigate to "Apps" menu
4. Choose "Add" to deploy your app
5. Browse to the `.appx` or `.msixbundle` file created in the `release-builds` folder
6. Click "Install"

### Method 2: Using Visual Studio

1. In Visual Studio, select "Debug" → "Start Without Debugging" (Ctrl+F5)
2. Select "Remote Machine" as the target
3. Enter your Xbox's IP address
4. Select "Universal (Unencrypted Protocol)" for Authentication Mode
5. Click "Deploy"

## Advanced Configuration

### Adjusting Package Settings

You can modify package settings by editing:
- `electron-builder.yml` - Main configuration for the build
- `electron/assets/AppxManifest.xml` - UWP package manifest

### Xbox-Specific Settings

For Xbox optimizations:
- Set `fullscreen: true` in electron-builder.yml
- Adjust controller input handling in your application
- Use Xbox-compatible UI patterns (large text, focus states, etc.)

## Troubleshooting

### Common Issues

1. **Certificate Issues**
   - Make sure your certificate is properly installed
   - Verify the publisher name in the manifest matches your certificate

2. **Deployment Failures**
   - Check Xbox Device Portal logs
   - Ensure Developer Mode is properly activated
   - Verify your Xbox has sufficient storage space

3. **Performance Issues**
   - Optimize rendering for Xbox hardware
   - Reduce texture sizes or graphical effects if needed

## Additional Resources

- [Xbox Dev Mode Documentation](https://learn.microsoft.com/en-us/windows/uwp/xbox-apps/getting-started)
- [UWP App Packaging Documentation](https://learn.microsoft.com/en-us/windows/uwp/packaging/)
- [Electron Documentation](https://www.electronjs.org/docs)
- [electron-builder Documentation](https://www.electron.build/)