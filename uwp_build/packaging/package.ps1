# SRB2P Xbox Packaging Script
# This script automates the entire build and packaging process for SRB2P on Xbox

param (
    [string]$SourceDir = ".",
    [string]$BuildDir = "build-uwp",
    [string]$Configuration = "Release",
    [switch]$Deploy = $false,
    [string]$XboxIP = "",
    [string]$CertPath = "",
    [string]$CertPass = "password"
)

# Display banner
Write-Host "=== SRB2P Xbox UWP Packaging Tool ===" -ForegroundColor Cyan
Write-Host "Preparing to build and package SRB2P for Xbox..." -ForegroundColor Cyan
Write-Host ""

# Ensure source directory exists
if (-not (Test-Path $SourceDir)) {
    Write-Host "Error: Source directory '$SourceDir' does not exist." -ForegroundColor Red
    exit 1
}

# Create build directory if it doesn't exist
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory: $BuildDir" -ForegroundColor Yellow
    New-Item -Path $BuildDir -ItemType Directory | Out-Null
}

# Navigate to build directory
Push-Location $BuildDir

try {
    # Step 1: Configure with CMake
    Write-Host "Configuring CMake build..." -ForegroundColor Green
    cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 "$SourceDir/uwp_build"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: CMake configuration failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # Step 2: Build the solution
    Write-Host "Building SRB2P for Xbox ($Configuration)..." -ForegroundColor Green
    cmake --build . --config $Configuration
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: Build failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # Step 3: Create APPX package
    Write-Host "Creating APPX package..." -ForegroundColor Green
    
    # Create AppPackages directory if it doesn't exist
    $AppPackagesDir = "AppPackages"
    if (-not (Test-Path $AppPackagesDir)) {
        New-Item -Path $AppPackagesDir -ItemType Directory | Out-Null
    }
    
    # Use MakeAppx.exe to create the package
    $MakeAppxExe = "$env:ProgramFiles(x86)\Windows Kits\10\bin\10.0.19041.0\x64\makeappx.exe"
    if (-not (Test-Path $MakeAppxExe)) {
        # Try to find makeappx.exe in other SDK versions
        $MakeAppxExe = Get-ChildItem -Path "$env:ProgramFiles(x86)\Windows Kits\10\bin" -Recurse -Filter "makeappx.exe" | 
                        Where-Object { $_.FullName -match "x64" } | 
                        Sort-Object -Property FullName -Descending | 
                        Select-Object -First 1 -ExpandProperty FullName
        
        if (-not $MakeAppxExe) {
            Write-Host "Error: Could not find makeappx.exe. Make sure Windows 10 SDK is installed." -ForegroundColor Red
            exit 1
        }
    }
    
    & $MakeAppxExe pack /d "$Configuration" /p "$AppPackagesDir\SRB2P.appx" /o
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: APPX package creation failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # Step 4: Sign the package
    Write-Host "Signing APPX package..." -ForegroundColor Green
    
    # Use provided certificate or create a self-signed one
    if (-not $CertPath -or -not (Test-Path $CertPath)) {
        Write-Host "No certificate provided, creating a self-signed certificate..." -ForegroundColor Yellow
        $CertPath = "SRB2P_Cert.pfx"
        
        # Create self-signed certificate
        New-SelfSignedCertificate -Type Custom -Subject "CN=Developer" -KeyUsage DigitalSignature `
            -FriendlyName "SRB2P Dev Cert" -CertStoreLocation "Cert:\CurrentUser\My" | 
            Export-PfxCertificate -FilePath $CertPath -Password (ConvertTo-SecureString -String $CertPass -Force -AsPlainText)
    }
    
    # Find signtool.exe
    $SignToolExe = "$env:ProgramFiles(x86)\Windows Kits\10\bin\10.0.19041.0\x64\signtool.exe"
    if (-not (Test-Path $SignToolExe)) {
        # Try to find signtool.exe in other SDK versions
        $SignToolExe = Get-ChildItem -Path "$env:ProgramFiles(x86)\Windows Kits\10\bin" -Recurse -Filter "signtool.exe" | 
                        Where-Object { $_.FullName -match "x64" } | 
                        Sort-Object -Property FullName -Descending | 
                        Select-Object -First 1 -ExpandProperty FullName
        
        if (-not $SignToolExe) {
            Write-Host "Error: Could not find signtool.exe. Make sure Windows 10 SDK is installed." -ForegroundColor Red
            exit 1
        }
    }
    
    & $SignToolExe sign /fd SHA256 /a /f $CertPath /p $CertPass "$AppPackagesDir\SRB2P.appx"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: APPX package signing failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # Step 5: Deploy to Xbox if requested
    if ($Deploy) {
        Write-Host "Deploying to Xbox..." -ForegroundColor Green
        
        if ([string]::IsNullOrEmpty($XboxIP)) {
            # Use transfer package method
            $TransferDir = "$env:USERPROFILE\AppData\Local\Packages\Microsoft.XboxDevices_8wekyb3d8bbwe\LocalState\DevelopmentFiles\TransferPackage"
            
            if (-not (Test-Path $TransferDir)) {
                Write-Host "Error: Transfer directory not found. Make sure Xbox Device Portal is installed." -ForegroundColor Red
                exit 1
            }
            
            Copy-Item -Path "$AppPackagesDir\SRB2P.appx" -Destination $TransferDir -Force
            Write-Host "Package copied to transfer directory. Open Dev Home on your Xbox to install." -ForegroundColor Yellow
        }
        else {
            # Use WDP for direct deployment
            Write-Host "Direct deployment to Xbox at $XboxIP is not implemented yet. Please use the transfer package method." -ForegroundColor Yellow
        }
    }
    
    # Display success message
    Write-Host ""
    Write-Host "Success! SRB2P has been successfully packaged for Xbox." -ForegroundColor Green
    Write-Host "Package location: $((Resolve-Path "$AppPackagesDir\SRB2P.appx").Path)" -ForegroundColor Green
    
    if ($Deploy) {
        Write-Host "The package has been prepared for deployment to your Xbox." -ForegroundColor Green
    }
    else {
        Write-Host "To deploy the package to your Xbox, run this script with the -Deploy switch." -ForegroundColor Yellow
    }

}
catch {
    Write-Host "Error: An unexpected error occurred:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
finally {
    # Return to the original directory
    Pop-Location
}