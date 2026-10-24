; Prueba completa Parte 2: Factorial recursivo con marco de pila
\\DATA 64
\\STACK 512

PUSH 5
CALL factorial
ADD SP, 4
MOV EBX, EAX ; EBX = 5! = 120
RET

factorial: PUSH BP
MOV BP, SP

MOV EAX, [BP+8]
CMP EAX, 1
JN caso_base
JZ caso_base

; Caso recursivo: N * factorial(N - 1)
SUB EAX, 1
PUSH EAX
CALL factorial
ADD SP, 4

MOV ECX, [BP+8]
MUL EAX, ECX
JMP fin_factorial

caso_base: MOV EAX, 1

fin_factorial: POP BP
RET
