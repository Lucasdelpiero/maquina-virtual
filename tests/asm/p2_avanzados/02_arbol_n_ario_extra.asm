; Prueba avanzada Parte 2: Recorrido y sumatoria recursiva en Arbol N-ario en Extra Segment (ES)
; Estructura de cada nodo N-ario:
;   +0: Valor (4 bytes)
;   +4: Cantidad de hijos K (4 bytes)
;   +8..(8 + 4*K): Arreglo de offsets a los K hijos en ES

\\DATA 64
\\EXTRA 512
\\STACK 512

; 1. Construccion del Arbol N-ario en Extra Segment
; Nodo Raiz en offset 0: valor 10, 3 hijos (offsets 24, 44, 64)
MOV l[ES+0], 10
MOV l[ES+4], 3
MOV l[ES+8], 24
MOV l[ES+12], 44
MOV l[ES+16], 64

; Nodo Hijo 1 en offset 24: valor 5, 2 hijos (offsets 84, 96)
MOV l[ES+24], 5
MOV l[ES+28], 2
MOV l[ES+32], 84
MOV l[ES+36], 96

; Nodo Hijo 2 en offset 44: valor 8, 0 hijos (hoja)
MOV l[ES+44], 8
MOV l[ES+48], 0

; Nodo Hijo 3 en offset 64: valor 12, 1 hijo (offset 108)
MOV l[ES+64], 12
MOV l[ES+68], 1
MOV l[ES+72], 108

; Hojas hijas de Nodo 1:
; Hoja offset 84: valor 2, 0 hijos
MOV l[ES+84], 2
MOV l[ES+88], 0
; Hoja offset 96: valor 3, 0 hijos
MOV l[ES+96], 3
MOV l[ES+100], 0

; Hoja hija de Nodo 3:
; Hoja offset 108: valor 7, 0 hijos
MOV l[ES+108], 7
MOV l[ES+112], 0

; 2. Llamada recursiva a sumar_arbol_n_ario(offset_raiz)
PUSH 0 ; Raiz en offset 0
CALL sumar_arbol_n_ario
ADD SP, 4

; Resultado esperado: 10 + 5 + 8 + 12 + 2 + 3 + 7 = 47
MOV EBX, EAX ; EBX = 47
RET

; Subrutina recursiva: sumar_arbol_n_ario(offset_nodo) -> EAX = suma del subarbol
sumar_arbol_n_ario: PUSH BP
MOV BP, SP
SUB SP, 8 ; [BP-4] = acumulador_suma, [BP-8] = indice_hijo_actual

MOV EDX, [BP+8] ; Offset del nodo actual
MOV EAX, ES
ADD EAX, EDX   ; EAX apunta fisicamente al nodo en ES

MOV ECX, l[EAX] ; ECX = valor del nodo actual
MOV [BP-4], ECX ; acumulador_suma = valor

MOV ECX, l[EAX+4] ; ECX = cantidad de hijos K
CMP ECX, 0
JZ fin_nodo_hoja ; Si no tiene hijos, termina y retorna acumulador_suma

MOV [BP-8], 0 ; indice_hijo = 0

bucle_hijos: MOV EDX, [BP-8]
MUL EDX, 4
ADD EDX, 8

MOV EAX, ES
ADD EAX, [BP+8]
ADD EAX, EDX ; EAX apunta al puntero del hijo i

MOV EDX, l[EAX] ; EDX = offset del hijo i en ES
PUSH EDX
CALL sumar_arbol_n_ario
ADD SP, 4

; Acumular retorno de la llamada en [BP-4]
ADD [BP-4], EAX

; Siguiente hijo
ADD [BP-8], 1
MOV EDX, [BP-8]

; Recalcular limite de hijos K
MOV EAX, ES
ADD EAX, [BP+8]
MOV ECX, l[EAX+4]

CMP EDX, ECX
JN bucle_hijos ; Mientras indice_hijo < K

fin_nodo_hoja: MOV EAX, [BP-4] ; Retorna sumatoria total
MOV SP, BP
POP BP
RET
