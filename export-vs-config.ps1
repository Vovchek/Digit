# Export Visual Studio Configuration
# This script exports VS workloads and components to a .vsconfig file

param(
    [string]$OutputPath = ".vsconfig"
)

Write-Host "Exporting Visual Studio configuration..." -ForegroundColor Cyan

$vsInstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe"

if (Test-Path $vsInstallerPath) {
    Write-Host "Found Visual Studio Installer at: $vsInstallerPath" -ForegroundColor Green
    
    # Export configuration
    & $vsInstallerPath export --config $OutputPath --quiet
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Configuration exported successfully to: $OutputPath" -ForegroundColor Green
        Write-Host ""
        Write-Host "To import on another computer:" -ForegroundColor Yellow
        Write-Host "  1. Copy the .vsconfig file to the target computer" -ForegroundColor White
        Write-Host "  2. Run Visual Studio Installer" -ForegroundColor White
        Write-Host "  3. Select 'Import configuration' or run:" -ForegroundColor White
        Write-Host "     vs_installer.exe --config .vsconfig" -ForegroundColor Cyan
    } else {
        Write-Host "Export failed with error code: $LASTEXITCODE" -ForegroundColor Red
    }
} else {
    Write-Host "Visual Studio Installer not found at expected path" -ForegroundColor Red
    Write-Host "Creating manual .vsconfig file..." -ForegroundColor Yellow
    
    # Create manual configuration
    $config = @{
        version = "1.0"
        components = @(
            "Microsoft.VisualStudio.Workload.NativeDesktop"
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
            "Microsoft.VisualStudio.Component.VC.CMake.Project"
            "Microsoft.VisualStudio.Component.VC.ATL"
            "Microsoft.VisualStudio.Component.VC.ATLMFC"
            "Microsoft.VisualStudio.Component.VC.TestAdapterForGoogleTest"
            "Microsoft.VisualStudio.Component.Windows10SDK.19041"
            "Microsoft.VisualStudio.Component.Git"
        )
    }
    
    $config | ConvertTo-Json | Set-Content -Path $OutputPath -Encoding UTF8
    Write-Host "Manual configuration created at: $OutputPath" -ForegroundColor Green
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
