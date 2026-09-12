; Test: Suma con desbordamiento (overflow) usando valores maximos
ldl eax, 0xFFFF
ldh eax, 0x7FFF
mov ebx, eax
add eax, ebx
stop
