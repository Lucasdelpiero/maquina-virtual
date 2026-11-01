# Script para ejecutar la maquina virtual (vmx.exe) exclusivamente sobre los binarios de la Parte 1 (cabecera V1)
param (
    [string]$VmxPath = "",
    [switch]$Disassembler,
    [switch]$Debug,
    [string]$Filter = "*"
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir = (Resolve-Path (Join-Path $scriptDir "..")).Path
$vmxDir = Join-Path $scriptDir "vmx"

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  VMX - EJECUTOR DE PRUEBAS PARTE 1 (CABECERA V1)" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan

# 1. Localizar ejecutable vmx.exe
$vmxCandidates = @(
    $VmxPath,
    (Join-Path $rootDir "bin\vmx.exe"),
    (Join-Path $rootDir "vmx.exe"),
    (Join-Path $rootDir "bin\Debug\proyecto-maquina-virtual.exe"),
    (Join-Path $rootDir "bin\Release\proyecto-maquina-virtual.exe"),
    (Join-Path (Get-Location) "vmx.exe"),
    (Join-Path (Get-Location) "bin\vmx.exe")
)

$vmxExe = $null
foreach ($candidate in $vmxCandidates) {
    if ($candidate -and (Test-Path $candidate)) {
        $vmxExe = (Resolve-Path $candidate).Path
        break
    }
}

if (-not $vmxExe) {
    Write-Host "ERROR: No se encontro vmx.exe. Compile el proyecto o especifique -VmxPath <ruta>" -ForegroundColor Red
    exit 1
}

Write-Host "Ejecutable VM: $vmxExe" -ForegroundColor Yellow
Write-Host "Directorio:    $vmxDir"
Write-Host "Filtro:        $Filter"
$flags = @()
if ($Disassembler) { $flags += "-d"; Write-Host "Flag activo:   -d (Disassembler)" -ForegroundColor Magenta }
if ($Debug)        { $flags += "-dev"; Write-Host "Flag activo:   -dev (Debug Logger)" -ForegroundColor Magenta }
Write-Host "--------------------------------------------------"

if (-not (Test-Path $vmxDir)) {
    Write-Host "ERROR: Directorio $vmxDir no existe. Ejecute primero traducir_tests.ps1" -ForegroundColor Red
    exit 1
}

# Obtener todos los archivos .vmx y filtrar aquellos cuya cabecera corresponde a la Version 1 (byte 5 == 1)
$todosVmx = Get-ChildItem -Path $vmxDir -Filter "*.vmx" -Recurse | Where-Object {
    ($_.Name -like "$Filter.vmx" -or $_.Name -like "$Filter")
}

$vmxFilesP1 = @()
foreach ($file in $todosVmx) {
    $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
    if ($bytes.Length -ge 6 -and $bytes[0] -eq 86 -and $bytes[1] -eq 77 -and $bytes[2] -eq 88 -and $bytes[3] -eq 50 -and $bytes[4] -eq 54) {
        $version = $bytes[5]
        if ($version -eq 1) {
            $vmxFilesP1 += $file
        }
    }
}

$total = $vmxFilesP1.Count
$exitosos = 0
$fallidos = 0

Write-Host "Binarios con cabecera V1 detectados: $total" -ForegroundColor Cyan
Write-Host "--------------------------------------------------"

foreach ($file in $vmxFilesP1) {
    $relPath = $file.FullName.Substring($vmxDir.Length).TrimStart("\", "/")

    # Ejecutar vmx.exe pasando entrada por stdin (por si el programa contiene SYS 1 / READ)
    $salida = $null
    $codigoSalida = 0
    try {
        $salida = ("0" | & $vmxExe $file.FullName @flags 2>&1)
        $codigoSalida = $LASTEXITCODE
    } catch {
        $salida = $_.Exception.Message
        $codigoSalida = 1
    }

    $esConflictivo = $relPath -like "conflictivos\*" -or $relPath -like "conflictivos/*"

    if ($esConflictivo) {
        if ($codigoSalida -ne 0) {
            Write-Host "  [OK]   $relPath (Excepcion esperada capturada)" -ForegroundColor Green
            if ($salida) {
                foreach ($line in $salida) {
                    Write-Host "         $line" -ForegroundColor DarkYellow
                }
            }
            $exitosos++
        } else {
            Write-Host "  [FAIL] $relPath (Debio fallar con excepcion pero termino con 0)" -ForegroundColor Red
            $fallidos++
        }
    } else {
        if ($codigoSalida -eq 0) {
            Write-Host "  [OK]   $relPath" -ForegroundColor Green
            if ($Disassembler -and $salida) {
                foreach ($line in $salida) {
                    Write-Host "         $line" -ForegroundColor DarkGray
                }
            }
            $exitosos++
        } else {
            Write-Host "  [FAIL] $relPath (Codigo de salida: $codigoSalida)" -ForegroundColor Red
            if ($salida) {
                foreach ($line in $salida) {
                    Write-Host "         $line" -ForegroundColor DarkRed
                }
            }
            $fallidos++
        }
    }
}

Write-Host "--------------------------------------------------"
Write-Host "RESUMEN EJECUCION PARTE 1: Total=$total | Exitosos=$exitosos | Fallidos=$fallidos" -ForegroundColor Cyan
Write-Host "=================================================="

if ($fallidos -gt 0) {
    exit 1
} else {
    exit 0
}
