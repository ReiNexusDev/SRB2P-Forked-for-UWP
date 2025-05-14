@echo off
echo Building SRB2P Xbox UWP Package...

rem Set environment variables for UWP/Xbox
set "ELECTRON_BUILDER_ALLOW_UNRESOLVED_DEPENDENCIES=true"

rem Install dependencies if needed
echo Installing dependencies...
call npm install

rem Build the application
echo Building the application...
call npm run build

rem Package for UWP
echo Packaging for UWP/Xbox...
npx electron-builder build --win --config electron-builder.yml

echo Done! Check the release-builds folder for your APPX package.
pause