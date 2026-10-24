; Prueba avanzada Parte 2: Inspeccion de Param Segment y argumentos de subrutina principal
; Al inicio:
;   [SP]   = RET (-1)
;   [SP+4] = argc
;   [SP+8] = *argv

\\DATA 64
\\STACK 128

; Leer argumentos del marco de inicio
MOV EAX, [SP+4] ; EAX = argc
MOV EBX, [SP+8] ; EBX = puntero argv

; Si argc == 0, terminar
CMP EAX, 0
JZ fin_prog

; Si argc > 0, leer el primer puntero de argv
MOV EDX, PS
ADD EDX, EBX ; EDX apunta al arreglo argv en Param Segment
MOV ECX, l[EDX] ; ECX = offset del primer string dentro de PS

; Leer el primer caracter del primer string
MOV EDX, PS
ADD EDX, ECX
MOV AL, b[EDX] ; AL = primer caracter (ej: 'p' de "prog.vmx")

fin_prog: RET
