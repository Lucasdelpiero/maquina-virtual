; Programa complejo 2: Calculo de factorial (5! = 120)
mov eax, 1
mov ecx, 5
loop_fact: mul eax, ecx
sub ecx, 1
cmp ecx, 1
jp loop_fact
stop
