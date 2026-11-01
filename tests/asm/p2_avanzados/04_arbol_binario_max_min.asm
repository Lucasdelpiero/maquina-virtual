; Prueba avanzada Parte 2: Busqueda de Minimo, Maximo y Conteo en Arbol Binario en Extra Segment
\\DATA 64
\\EXTRA 512
\\STACK 512

; Construccion del Arbol Binario de Busqueda en ES
; Nodo 0 (raiz): valor 50, izq -> 12, der -> 24
MOV l[ES+0], 50
MOV l[ES+4], 12
MOV l[ES+8], 24

; Nodo 1 (offset 12): valor 30, izq -> 36, der -> 48
MOV l[ES+12], 30
MOV l[ES+16], 36
MOV l[ES+20], 48

; Nodo 2 (offset 24): valor 85, izq -> 60, der -> 72
MOV l[ES+24], 85
MOV l[ES+28], 60
MOV l[ES+32], 72

; Nodo 3 (offset 36): valor 15 (hoja)
MOV l[ES+36], 15
MOV l[ES+40], -1
MOV l[ES+44], -1

; Nodo 4 (offset 48): valor 45 (hoja)
MOV l[ES+48], 45
MOV l[ES+52], -1
MOV l[ES+56], -1

; Nodo 5 (offset 60): valor 70 (hoja)
MOV l[ES+60], 70
MOV l[ES+64], -1
MOV l[ES+68], -1

; Nodo 6 (offset 72): valor 95 (hoja)
MOV l[ES+72], 95
MOV l[ES+76], -1
MOV l[ES+80], -1

; 1. Buscar valor minimo en BST (debe ser 15)
PUSH 0 ; offset raiz
CALL bst_minimo
ADD SP, 4
MOV EBX, EAX ; EBX = 15

; 2. Buscar valor maximo en BST (debe ser 95)
PUSH 0 ; offset raiz
CALL bst_maximo
ADD SP, 4
MOV EFX, EAX ; EFX = 95

; 3. Contar total de nodos en BST (debe ser 7)
PUSH 0 ; offset raiz
CALL bst_contar_nodos
ADD SP, 4
MOV EDX, EAX ; EDX = 7

RET

; Subrutina bst_minimo: sigue punteros izquierdos hasta que sea -1
bst_minimo: PUSH BP
MOV BP, SP
MOV EDX, [BP+8] ; offset nodo actual

bucle_min: MOV EAX, ES
ADD EAX, EDX
MOV ECX, l[EAX+4] ; puntero hijo izquierdo
CMP ECX, -1
JZ fin_min
MOV EDX, ECX
JMP bucle_min

fin_min: MOV EAX, l[EAX] ; retorna valor en EAX
POP BP
RET

; Subrutina bst_maximo: sigue punteros derechos hasta que sea -1
bst_maximo: PUSH BP
MOV BP, SP
MOV EDX, [BP+8] ; offset nodo actual

bucle_max: MOV EAX, ES
ADD EAX, EDX
MOV ECX, l[EAX+8] ; puntero hijo derecho
CMP ECX, -1
JZ fin_max
MOV EDX, ECX
JMP bucle_max

fin_max: MOV EAX, l[EAX] ; retorna valor en EAX
POP BP
RET

; Subrutina recursiva bst_contar_nodos(offset_nodo) -> EAX
bst_contar_nodos: PUSH BP
MOV BP, SP
SUB SP, 8 ; [BP-4] = hijo_der, [BP-8] = res_izq

MOV EDX, [BP+8]
CMP EDX, -1
JZ caso_base_contar

MOV EAX, ES
ADD EAX, EDX
MOV ECX, l[EAX+4] ; izq
MOV EDX, l[EAX+8] ; der
MOV [BP-4], EDX   ; guardar der

PUSH ECX
CALL bst_contar_nodos
ADD SP, 4
MOV [BP-8], EAX   ; guardar res izq

PUSH [BP-4]
CALL bst_contar_nodos
ADD SP, 4

ADD EAX, [BP-8]
ADD EAX, 1
JMP fin_contar

caso_base_contar: MOV EAX, 0

fin_contar: MOV SP, BP
POP BP
RET
