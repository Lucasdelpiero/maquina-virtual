; Prueba completa Parte 2: Comparacion de strings (strcmp) en Assembler
\\DATA 64
\\STACK 256

CADENA_A EQU "Arquitectura"
CADENA_B EQU "Arquitectura"
CADENA_C EQU "Arqui2026"

; Comparacion 1: CADENA_A vs CADENA_B (deben ser iguales -> EAX = 0)
MOV EDX, KS
ADD EDX, CADENA_A
PUSH EDX
MOV EDX, KS
ADD EDX, CADENA_B
PUSH EDX
CALL comparar_strings
ADD SP, 8
MOV EBX, EAX ; EBX = 0 (iguales)

; Comparacion 2: CADENA_A vs CADENA_C (deben ser distintas -> EAX != 0)
MOV EDX, KS
ADD EDX, CADENA_A
PUSH EDX
MOV EDX, KS
ADD EDX, CADENA_C
PUSH EDX
CALL comparar_strings
ADD SP, 8
MOV ECX, EAX ; ECX != 0 (distintas)

RET

; Subrutina: comparar_strings(ptr1, ptr2)
; Retorna 0 si son iguales, o la diferencia en el primer caracter distinto
comparar_strings: PUSH BP
MOV BP, SP
MOV EDX, [BP+12] ; ptr1
MOV EEX, [BP+8]  ; ptr2

bucle_cmp: MOV AL, b[EDX]
MOV AH, b[EEX]

CMP AL, AH
JNZ fin_distintos

CMP AL, 0
JZ fin_iguales

ADD EDX, 1
ADD EEX, 1
JMP bucle_cmp

fin_iguales: MOV EAX, 0
JMP fin_sub

fin_distintos: SUB AL, AH
MOV EAX, AL

fin_sub: POP BP
RET
