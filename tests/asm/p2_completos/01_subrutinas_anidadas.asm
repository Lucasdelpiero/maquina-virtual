; Prueba completa Parte 2: Subrutinas anidadas (main -> func_a -> func_b)
\\DATA 128
\\STACK 256

CALL func_a
MOV EDX, EAX ; Copia resultado final a EDX
RET

func_a: PUSH EBX
MOV EBX, 10
CALL func_b
ADD EAX, EBX ; EAX = resultado de func_b + 10
POP EBX
RET

func_b: MOV EAX, 25
RET
