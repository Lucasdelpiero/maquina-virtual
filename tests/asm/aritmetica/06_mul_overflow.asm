; Test: Multiplicacion con overflow y carry (caso especificacion)
ldl eax, 0x0003
ldh eax, 0x4000
mul eax, 4
stop
