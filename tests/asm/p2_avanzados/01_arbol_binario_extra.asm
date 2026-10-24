; Prueba avanzada Parte 2: Arbol Binario de Busqueda en Extra Segment (ES) y busqueda recursiva
; Estructura de cada nodo (12 bytes en ES):
;   +0: Valor (4 bytes)
;   +4: Offset hijo izquierdo (-1 si no tiene)
;   +8: Offset hijo derecho (-1 si no tiene)

\\DATA 64
\\EXTRA 512
\\STACK 512

; Construccion estatica del arbol en ES
; Nodo 0 (raiz): valor 50, izq -> Nodo 1 (offset 12), der -> Nodo 2 (offset 24)
MOV l[ES+0], 50
MOV l[ES+4], 12
MOV l[ES+8], 24

; Nodo 1 (offset 12): valor 30, izq -> Nodo 3 (offset 36), der -> Nodo 4 (offset 48)
MOV l[ES+12], 30
MOV l[ES+16], 36
MOV l[ES+20], 48

; Nodo 2 (offset 24): valor 70, izq -> Nodo 5 (offset 60), der -> -1
MOV l[ES+24], 70
MOV l[ES+28], 60
MOV l[ES+32], -1

; Nodo 3 (offset 36): valor 20, hojas (-1, -1)
MOV l[ES+36], 20
MOV l[ES+40], -1
MOV l[ES+44], -1

; Nodo 4 (offset 48): valor 40, hojas (-1, -1)
MOV l[ES+48], 40
MOV l[ES+52], -1
MOV l[ES+56], -1

; Nodo 5 (offset 60): valor 60, hojas (-1, -1)
MOV l[ES+60], 60
MOV l[ES+64], -1
MOV l[ES+68], -1

; 1. Busqueda recursiva de 60 (debe encontrarlo -> EAX = 1)
PUSH 60 ; Valor buscado
PUSH 0  ; Offset raiz
CALL buscar_bst
ADD SP, 8
MOV EBX, EAX ; EBX = 1

; 2. Busqueda recursiva de 99 (no existe -> EAX = 0)
PUSH 99 ; Valor buscado
PUSH 0  ; Offset raiz
CALL buscar_bst
ADD SP, 8
MOV ECX, EAX ; ECX = 0

RET

; Subrutina: buscar_bst(offset_nodo, valor) -> 1 si existe, 0 si no
buscar_bst: PUSH BP
MOV BP, SP

MOV EDX, [BP+8] ; Offset del nodo actual
CMP EDX, -1
JZ no_encontrado ; Si puntero es nulo (-1), no esta

MOV EAX, ES
ADD EAX, EDX ; EAX apunta al nodo en ES

MOV ECX, l[EAX] ; ECX = valor del nodo actual
MOV EDX, [BP+12] ; EDX = valor buscado

CMP EDX, ECX
JZ encontrado
JN buscar_izq

; Buscar en subarbol derecho: offset en [EAX+8]
MOV EDX, l[EAX+8]
PUSH [BP+12]
PUSH EDX
CALL buscar_bst
ADD SP, 8
JMP fin_buscar

buscar_izq: MOV EDX, l[EAX+4]
PUSH [BP+12]
PUSH EDX
CALL buscar_bst
ADD SP, 8
JMP fin_buscar

encontrado: MOV EAX, 1
JMP fin_buscar

no_encontrado: MOV EAX, 0

fin_buscar: POP BP
RET
