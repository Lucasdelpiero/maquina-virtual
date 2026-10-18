; Test: JNZ no debe saltar si el resultado es cero (Z=1)
mov eax, 0
jnz no_debe_saltar
mov ebx, 42
jmp fin
no_debe_saltar: mov ebx, 99
fin: stop
