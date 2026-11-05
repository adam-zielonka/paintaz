# App-local MSVC release CRT for the portable ZIP (no administrator install).
param([Parameter(Mandatory=$true)][ValidateSet('x86_64', 'arm64')][string]$Architecture)
$ErrorActionPreference = 'Stop'
$crtArch = if ($Architecture -eq 'x86_64') { 'x64' } else { 'arm64' }
$redistRoot = $env:VCToolsRedistDir
if (-not $redistRoot) { throw 'VCToolsRedistDir is missing; initialize the MSVC developer shell first.' }
$crtDir = Get-ChildItem -Path (Join-Path $redistRoot $crtArch) -Directory -Filter 'Microsoft.VC*.CRT' |
    Sort-Object Name -Descending | Select-Object -First 1
if (-not $crtDir) { throw "MSVC release CRT for $crtArch is missing under $redistRoot" }
Copy-Item -Path (Join-Path $crtDir.FullName '*.dll') -Destination 'build/package/paintAZ/'
foreach ($required in @('msvcp140.dll', 'vcruntime140.dll')) {
    if (-not (Test-Path "build/package/paintAZ/$required")) { throw "Missing portable CRT: $required" }
}
# Verify every DLL, including plugins and CRT, belongs to the target architecture.
Get-ChildItem -Path 'build/package/paintAZ' -Filter '*.dll' -Recurse | ForEach-Object {
    python .github/scripts/check-architecture.py $_.FullName $Architecture
    if ($LASTEXITCODE -ne 0) { throw "Wrong DLL architecture: $($_.FullName)" }
}
