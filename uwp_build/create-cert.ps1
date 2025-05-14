# Script to create a self-signed certificate for UWP/Xbox development
Write-Host "Creating self-signed certificate for UWP development..." -ForegroundColor Green

$publisherName = "CN=ReiNexusDev"
$certPath = ".\certificate.pfx"
$password = "SRB2PDev" # You should change this in a real scenario!

try {
    # Create a self-signed certificate
    Write-Host "Generating certificate..." -ForegroundColor Cyan
    $cert = New-SelfSignedCertificate -Type Custom `
                                     -Subject $publisherName `
                                     -KeyUsage DigitalSignature `
                                     -FriendlyName "SRB2P Xbox UWP Development Certificate" `
                                     -CertStoreLocation "Cert:\CurrentUser\My" `
                                     -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}")

    # Export the certificate
    Write-Host "Exporting certificate to $certPath..." -ForegroundColor Cyan
    $pwd = ConvertTo-SecureString -String $password -Force -AsPlainText
    Export-PfxCertificate -Cert "Cert:\CurrentUser\My\$($cert.Thumbprint)" -FilePath $certPath -Password $pwd

    # Trust the certificate (only needed for local testing, not for Xbox)
    Write-Host "Adding certificate to trusted store..." -ForegroundColor Cyan
    Import-PfxCertificate -FilePath $certPath -CertStoreLocation "Cert:\CurrentUser\TrustedPeople" -Password $pwd

    Write-Host "Certificate created successfully!" -ForegroundColor Green
    Write-Host "Certificate Password: $password" -ForegroundColor Yellow
    Write-Host "IMPORTANT: In a production environment, use a proper certificate and secure password!" -ForegroundColor Yellow
}
catch {
    Write-Host "Error creating certificate: $_" -ForegroundColor Red
}

Write-Host "Process complete. Press any key to exit..." -ForegroundColor Green
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")