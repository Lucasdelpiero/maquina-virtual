; Programa de ejemplo completo: contar la cantidad de bits en 1
inicio: mov eax, 0b01 ; seteo para leer en decimal
mov edx, DS ; guardar en el data segment
add edx, 4 ; en la posicion 1
ldh ecx, 0x04 ; leer valores de 4 bytes
ldl ecx, 0x01 ; leer un solo valor
sys 0x1 ; llamada al sistema para leer

xor ac, ac ; reseteo el acumulador (ac = 0)
mov eax, [edx] ; copio 4 bytes de memoria a registro

otro: cmp eax, 0 ; comparo con cero
jz fin ; si es cero termine
jnn sigue ; si no es negativo salta
add ac, 1 ; si es negativo acumula 1

sigue: shl eax, 1 ; desplazo un bit a la izquierda
jmp otro ; continua el bucle

fin: add edx, 4 ; incremento para usar otra posicion
mov [edx], ac ; copia a memoria el ac
mov eax, 0b01 ; seteo para escribir decimal
ldh ecx, 0x04 ; escribir valores de 4 bytes
ldl ecx, 0x01 ; escribir un solo valor
sys 0x2 ; llamada al sistema para imprimir
stop ; detiene la ejecucion
