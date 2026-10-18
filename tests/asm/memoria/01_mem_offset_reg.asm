; Test: Acceso a memoria con registro base y offset positivo [reg+desp]
mov edx, ds
add edx, 4
mov [edx+8], 999
mov eax, [edx+8]
stop
