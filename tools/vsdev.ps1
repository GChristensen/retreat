# Runs a command inside the Visual Studio x64 developer environment (cl, cmake, ninja,
# vcpkg). Does nothing extra when it is already active, e.g. in a VS Developer PowerShell.
#   powershell -File tools/vsdev.ps1 cmake --preset win-msvc-release

$ErrorActionPreference = 'Stop'

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) { throw "vswhere.exe not found; is Visual Studio installed?" }

    $vs = & $vswhere -latest -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vs) { throw "No Visual Studio with the C++ x64 tools found" }

    $location = Get-Location
    & (Join-Path $vs 'Common7\Tools\Launch-VsDevShell.ps1') -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null
    Set-Location $location
}

if ($args.Count -eq 0) { return }

$command, $rest = $args
& $command @rest
exit $LASTEXITCODE
