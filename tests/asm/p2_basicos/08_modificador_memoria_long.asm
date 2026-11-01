; Prueba basica Parte 2: Modificador de memoria long l[...]
LDL EAX, 0x3344
LDH EAX, 0x1122
MOV l[DS+0], EAX
MOV EBX, l[DS+0]
RET
