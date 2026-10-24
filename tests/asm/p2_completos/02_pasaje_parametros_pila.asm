; Prueba completa Parte 2: Pasaje de parametros a subrutina via pila
\\DATA 64
\\STACK 256

PUSH 15
PUSH 27
CALL sumar
ADD SP, 8
MOV EBX, EAX ; EBX = 42
RET

sumar: PUSH BP
MOV BP, SP
MOV EAX, [BP+8]
ADD EAX, [BP+12]
POP BP
RET
