; Caso conflictivo Parte 2: Desbordamiento de pila (Stack overflow)
\\DATA 32
\\STACK 32

; La pila tiene 32 bytes de capacidad. La inicializacion de main ocupa 12 bytes.
; Hacemos push en bucle infinito hasta saturar y provocar Stack overflow
bucle_push: PUSH 1111
JMP bucle_push
