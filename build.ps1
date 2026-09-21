<#
.SYNOPSIS
    CubeForge TogetherSpawn Build Script (MSVC x64) - CMake Presets Automation
.DESCRIPTION
    Script de automação para compilação unificada do mod TogetherSpawn com MSVC x64 e instalação automática.
    Utiliza CMakePresets.json do CMake 3.25+.
.EXAMPLE
    .\build.ps1
    .\build.ps1 -BuildType Release
    .\build.ps1 -Target clean
    .\build.ps1 -InstallPath "C:\Program Files (x86)\Steam\steamapps\common\Cube World"
#>

[CmdletBinding()]
param (
    [ValidateSet("all", "mod", "clean")]
    [string]$Target = "mod",

    [ValidateSet("Release", "Debug", "RelWithDebInfo")]
    [string]$BuildType = "Release",

    [string]$InstallPath = ""
)

$ErrorActionPreference = "Stop"

if ($Target -eq "clean") {
    if (Test-Path "build") {
        Write-Host "Limpando diretório build/..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force "build"
    }
    if (Test-Path "dist") {
        Write-Host "Limpando diretório dist/..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force "dist"
    }
    Write-Host "Limpeza concluída com sucesso." -ForegroundColor Green
    exit 0
}

# 1. Detectar e carregar ambiente MSVC x64 se necessário
if ($env:VSCMD_ARG_TGT_ARCH -ne "x64" -or -not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vcvars = $null
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -prerelease -property installationPath
        if ($vsPath -and (Test-Path "$vsPath\VC\Auxiliary\Build\vcvars64.bat")) {
            $vcvars = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
        }
    }
    if (-not $vcvars) {
        $candidate = Get-ChildItem -Path "C:\Program Files\Microsoft Visual Studio", "C:\Program Files (x86)\Microsoft Visual Studio" -Filter "vcvars64.bat" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($candidate) { $vcvars = $candidate.FullName }
    }
    if ($vcvars) {
        Write-Host "Inicializando ambiente MSVC x64: $vcvars" -ForegroundColor Cyan
        $envVars = cmd.exe /c "call `"$vcvars`" >nul 2>&1 && set"
        foreach ($line in $envVars) {
            if ($line -match "^(.*?)=(.*)$") {
                $varName = $matches[1]
                $varVal = $matches[2]
                if ($varName -ieq "PATH") {
                    $env:Path = $varVal
                } else {
                    [System.Environment]::SetEnvironmentVariable($varName, $varVal, "Process")
                }
            }
        }
    } else {
        Write-Warning "vcvars64.bat não encontrado automaticamente. Certifique-se de que o compilador MSVC está no PATH."
    }
}

# 2. Determinar Preset baseado em BuildType
$presetName = switch ($BuildType) {
    "Debug"          { "windows-debug" }
    "RelWithDebInfo" { "windows-relwithdebinfo" }
    Default          { "windows-release" }
}

# Remove dirty cmake cache if try_compile had failed previously
$presetBuildDir = "build/$presetName"
if (Test-Path "$presetBuildDir/CMakeCache.txt") {
    $cacheContent = Get-Content "$presetBuildDir/CMakeCache.txt" -Raw
    if ($cacheContent -match "CMAKE_C_COMPILER-FAILED" -or $cacheContent -match "CMAKE_CXX_COMPILER-FAILED") {
        Write-Host "Detectado cache corrompido em $presetBuildDir, limpando..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force $presetBuildDir
    }
}

Write-Host "==> Configurando CMake com Preset: $presetName" -ForegroundColor Cyan
cmake --preset $presetName
if ($LASTEXITCODE -ne 0) {
    Write-Error "A configuração do CMake falhou."
}

Write-Host "==> Compilando mod TogetherSpawn..." -ForegroundColor Cyan
cmake --build --preset $presetName --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Error "A compilação falhou com código de erro $LASTEXITCODE."
}

# 3. Instalação opcional no jogo
if ($InstallPath -ne "") {
    if (-not (Test-Path $InstallPath)) {
        Write-Error "Diretório de instalação não encontrado: $InstallPath"
    }

    $modsDir = if (Test-Path (Join-Path $InstallPath "Mods")) {
        Join-Path $InstallPath "Mods"
    } else {
        $created = New-Item -ItemType Directory -Path (Join-Path $InstallPath "Mods") -Force
        $created.FullName
    }

    Write-Host "==> Instalando DLL em $modsDir..." -ForegroundColor Magenta

    $builtDll = Get-ChildItem -Path "dist/Mods", "build/$presetName" -Filter "*.dll" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1

    if ($builtDll) {
        Copy-Item -Path $builtDll.FullName -Destination $modsDir -Force
        Write-Host " [OK] Mod copiado com sucesso para: $modsDir\$($builtDll.Name)" -ForegroundColor Green
    } else {
        Write-Warning "Nenhum arquivo DLL foi encontrado para cópia."
    }
}

Write-Host "Build concluído com sucesso!" -ForegroundColor Green
