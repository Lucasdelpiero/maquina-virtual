; Prueba completa Parte 2: Constante string y SYS 4 (STRING WRITE)
\\DATA 64
\\STACK 128

SALUDO EQU "Hola mundo desde Const Segment!\n"
DESPEDIDA EQU "Fin de prueba\n"

MOV EDX, KS
ADD EDX, SALUDO
SYS 4

MOV EDX, KS
ADD EDX, DESPEDIDA
SYS 4

MOV EAX, 1
RET
