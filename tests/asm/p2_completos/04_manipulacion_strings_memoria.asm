; Prueba completa Parte 2: Copia de string de KS a DS byte a byte y calculo de longitud
\\DATA 256
\\STACK 128

ORIGEN EQU "Arquitectura2026"

MOV ECX, 0 ; Contador de longitud y offset

bucle_copia: MOV EDX, KS
ADD EDX, ORIGEN
ADD EDX, ECX
MOV AL, b[EDX] ; Lee byte de Const Segment

MOV EDX, DS
ADD EDX, ECX
MOV b[EDX], AL ; Escribe byte en Data Segment

CMP AL, 0
JZ fin_copia
ADD ECX, 1
JMP bucle_copia

fin_copia: MOV EAX, ECX ; EAX tendra la longitud (16)
RET
