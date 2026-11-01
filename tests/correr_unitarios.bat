@echo off
gcc -Wall -Wextra -O2 -Isrc tests/unitarios/main_test.c tests/unitarios/test_helpers.c tests/unitarios/test_errores.c tests/unitarios/test_logger.c tests/unitarios/test_memoria.c tests/unitarios/test_operandos.c tests/unitarios/test_operadores.c tests/unitarios/test_decodificador.c tests/unitarios/test_param_segment.c tests/unitarios/test_sys.c tests/unitarios/test_vmi.c tests/unitarios/test_inicializador.c tests/unitarios/test_disassembler.c tests/unitarios/test_depurador.c tests/unitarios/test_vmx.c src/errores.c src/logger.c src/memoria.c src/operandos.c src/operadores.c src/decodificador.c src/param_segment.c src/sys.c src/vmi.c src/inicializador.c src/disassembler.c src/depurador.c src/vmx.c -o tests/runner_unitarios.exe
if %ERRORLEVEL% EQU 0 (
    tests\runner_unitarios.exe
)
