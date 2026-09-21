; Test: SWAP entre dos operandos de memoria [mem] <-> [mem]
mov [0], 111
mov [4], 222
swap [0], [4]
mov eax, [0]
mov ebx, [4]
stop
