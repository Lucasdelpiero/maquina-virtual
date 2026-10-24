; Prueba completa Parte 2: Algoritmo dig_max oficial de la catedra (recursivo con variables locales)
\\DATA 64
\\STACK 512

PUSH 3842
CALL DIR_MAX
ADD SP, 4
MOV EBX, EAX ; EBX = 8 (mayor digito de 3842)
RET

DIR_MAX: PUSH BP
MOV BP, SP
SUB SP, 4 ; ult en [BP-4]
SUB SP, 4 ; max en [BP-8]
PUSH AC

MOV EAX, [BP+8]
CMP EAX, 10
JN FIN_DM

DIV EAX, 10
MOV [BP-4], AC
PUSH EAX

CALL DIR_MAX
ADD SP, 4

MOV [BP-8], EAX

CMP [BP-4], [BP-8]
JN MAX_DM
MOV EAX, [BP-4]
JMP FIN_DM

MAX_DM: MOV EAX, [BP-8]

FIN_DM: POP AC
MOV SP, BP
POP BP
RET
