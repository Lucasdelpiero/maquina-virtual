# Script de automatizacion para traducir programas .asm a .vmx con vmt.exe
param (
    [string]$VmtPath = ""
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$asmDir = Join-Path $scriptDir "asm"
$vmxDir = Join-Path $scriptDir "vmx"

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  VMX - TRADUCTOR AUTOMATICO DE PRUEBAS (.asm -> .vmx)" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan

# 1. Localizar ejecutable vmt.exe
$vmtCandidates = @(
    $VmtPath,
    (Join-Path $scriptDir "..\vmt.exe"),
    (Join-Path $scriptDir "..\..\Maquina-Virtual\vmt.exe"),
    (Join-Path (Get-Location) "vmt.exe"),
    "C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual\vmt.exe"
)

$vmtExe = $null
foreach ($candidate in $vmtCandidates) {
    if ($candidate -and (Test-Path $candidate)) {
        $vmtExe = (Resolve-Path $candidate).Path
        break
    }
}

if (-not $vmtExe) {
    Write-Host "ERROR: No se encontro vmt.exe. Especifica la ruta con -VmtPath <ruta>" -ForegroundColor Red
    exit 1
}

Write-Host "Usando traductor: $vmtExe" -ForegroundColor Yellow
Write-Host "Buscando en:      $asmDir"
Write-Host "Destino:          $vmxDir"
Write-Host "--------------------------------------------------"

if (-not (Test-Path $asmDir)) {
    Write-Host "No se encontro el directorio de fuentes $asmDir" -ForegroundColor Yellow
    exit 0
}

$asmFiles = Get-ChildItem -Path $asmDir -Filter "*.asm" -Recurse

$total = $asmFiles.Count
$traducidos = 0
$alDia = 0
$fallidos = 0

foreach ($file in $asmFiles) {
    # Obtener ruta relativa respecto a $asmDir
    $relPath = $file.FullName.Substring($asmDir.Length).TrimStart("\", "/")
    $relVmxPath = [System.IO.Path]::ChangeExtension($relPath, ".vmx")
    $targetVmx = Join-Path $vmxDir $relVmxPath
    $targetFolder = Split-Path -Parent $targetVmx

    if (-not (Test-Path $targetFolder)) {
        New-Item -Path $targetFolder -ItemType Directory -Force | Out-Null
    }

    # Verificar si ya esta al dia (timestamp)
    if ((Test-Path $targetVmx) -and ((Get-Item $targetVmx).LastWriteTime -ge $file.LastWriteTime)) {
        Write-Host "  [SKIP] $relPath (al dia)" -ForegroundColor DarkGray
        $alDia++
        continue
    }

    # Traducir con vmt.exe
    & $vmtExe $file.FullName $targetVmx -o
    if ($LASTEXITCODE -eq 0 -and (Test-Path $targetVmx)) {
        Write-Host "  [OK]   $relPath -> $relVmxPath" -ForegroundColor Green
        $traducidos++
    } else {
        Write-Host "  [FAIL] $relPath" -ForegroundColor Red
        $fallidos++
    }
}

Write-Host "--------------------------------------------------"
Write-Host "RESUMEN: Total=$total | Traducidos=$traducidos | Al dia=$alDia | Fallidos=$fallidos" -ForegroundColor Cyan
Write-Host "=================================================="
