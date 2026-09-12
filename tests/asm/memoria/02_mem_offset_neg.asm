; Test: Acceso a memoria con registro base y offset negativo [reg-desp]
mov edx, ds
add edx, 16
mov [edx-4], 777
mov eax, [edx-4]
stop
