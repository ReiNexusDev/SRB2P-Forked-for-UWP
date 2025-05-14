# SRB2P Xbox UWP Package Builder
Write-Host "Building SRB2P Xbox UWP Package..." -ForegroundColor Green

# Set environment variables for UWP/Xbox
$env:ELECTRON_BUILDER_ALLOW_UNRESOLVED_DEPENDENCIES = "true"

# Check for required tools
if (-not (Get-Command "npm" -ErrorAction SilentlyContinue)) {
    Write-Host "Node.js and npm are required but not found. Please install them first." -ForegroundColor Red
    exit 1
}

# Install dependencies if needed
Write-Host "Installing dependencies..." -ForegroundColor Cyan
npm install

# Check if there's a certificate for signing
$certPath = ".\certificate.pfx"
if (Test-Path $certPath) {
    Write-Host "Certificate found. Package will be signed." -ForegroundColor Green
    $env:CSC_LINK = $certPath
    # Note: You'd need to set CSC_KEY_PASSWORD if your certificate has a password
}
else {
    Write-Host "No certificate found. Package will be unsigned (for testing only)." -ForegroundColor Yellow
}

# Build the application
Write-Host "Building the application..." -ForegroundColor Cyan
npm run build

# Package for UWP
Write-Host "Packaging for UWP/Xbox..." -ForegroundColor Cyan
try {
    npx electron-builder build --win --config electron-builder.yml
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Success! Check the release-builds folder for your APPX package." -ForegroundColor Green
    } else {
        Write-Host "Packaging failed with exit code $LASTEXITCODE" -ForegroundColor Red
    }
}
catch {
    Write-Host "Error occurred during packaging: $_" -ForegroundColor Red
}

Write-Host "Process complete. Press any key to exit..." -ForegroundColor Green
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")