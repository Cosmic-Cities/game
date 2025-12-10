#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Setup script for CosmicCities build environment
.DESCRIPTION
    Initializes Axmol dependencies and prepares build environment
#>

param(
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$axmolDir = Join-Path $scriptDir "axmol"

if ($Verbose) {
    Write-Host "Script directory: $scriptDir"
    Write-Host "Axmol directory: $axmolDir"
}

# Check if axmol setup script exists
$setupScript = Join-Path $axmolDir "1k\1kiss.ps1"
if (-not (Test-Path $setupScript)) {
    Write-Error "Setup script not found: $setupScript"
    exit 1
}

Write-Host "Running Axmol setup..." -ForegroundColor Cyan

# Run the setup script with setupOnly flag
try {
    & $setupScript -setupOnly
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Axmol setup failed with exit code: $LASTEXITCODE"
        exit 1
    }
} catch {
    Write-Error "Failed to run Axmol setup: $_"
    exit 1
}

Write-Host "Setup completed successfully!" -ForegroundColor Green
exit 0
