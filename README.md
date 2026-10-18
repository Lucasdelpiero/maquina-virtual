# Maquina Virtual (Vmx)

Maquina virtual para ejecutar archivos binarios, deben tener extension .vmx

## Ejemplo de ejecucion

./bin/Release/vmx.exe programa.vmx

## Ejemplo Powershell:

.\vmx.exe programa.vmx

## Parametros adicionales

-d : Disassembler
Desensambla todo el segmento codigo o code segment antes de correr el programa

-dev : Muestra todos los pasos de la ejecucion, util para debuggear, entrar errores mas rapidos, etc.

Se pueden usar combinados.