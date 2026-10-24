; Caso conflictivo Parte 2: Extraccion de pila vacia (Stack underflow)
\\DATA 32
\\STACK 64

; La pila inicia con 3 elementos (RET, argc, argv = 12 bytes)
POP EAX ; 1er pop (extrae RET)
POP EBX ; 2do pop (extrae argc)
POP ECX ; 3er pop (extrae argv - pila queda vacia al limite)
POP EDX ; 4to pop -> Provoca Stack underflow
RET
