; Test conflictivo: Acceso fuera de limite del segmento de datos (fallo de segmento)
mov edx, ds
add edx, 16384
mov [edx], 99
stop
