; Prueba basica Parte 2: PUSH desde memoria y POP a memoria
MOV [DS+0], 555
PUSH [DS+0]
POP [DS+4]
MOV EAX, [DS+4]
RET
