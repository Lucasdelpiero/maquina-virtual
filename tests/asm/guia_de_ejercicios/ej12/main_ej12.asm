\\DATA 1024
\\STACK 1024

JMP main

\\INCLUDE "cant_caminos.asm"

; =========================================================================
; PROGRAMA PRINCIPAL DE PRUEBA: EJERCICIO 12
; Digrafo aciclico con 5 vertices (0, 1, 2, 3, 4):
; Aristas:
;   0 -> 1
;   0 -> 2
;   1 -> 3
;   1 -> 4
;   2 -> 3
;   3 -> 4
;
; Caminos de origen 0 a destino 4:
;   Camino 1: 0 -> 1 -> 4
;   Camino 2: 0 -> 1 -> 3 -> 4
;   Camino 3: 0 -> 2 -> 3 -> 4
; Total caminos esperados = 3.
; Devuelve en EAX la cantidad de caminos (debe ser 3).
; =========================================================================

main:           MOV BP, SP

                ; ---------------------------------------------------------
                ; 1. Inicializar Matriz de Adyacencia 5x5 en 0
                ; Matriz ubicada en DS+8 (5x5 = 25 celdas * 4 = 100 bytes: DS+8 .. DS+107)
                ; ---------------------------------------------------------
                MOV ECX, 0
limpiar_mat:    MOV EDX, DS
                ADD EDX, 8
                ADD EDX, ECX
                MOV l[EDX], 0
                ADD ECX, 4
                CMP ECX, 100
                JN limpiar_mat

                ; ---------------------------------------------------------
                ; 2. Cargar Aristas del Grafo en la Matriz
                ; Offset = 8 + (fila * 5 + columna) * 4
                ; ---------------------------------------------------------
                ; 0 -> 1 : offset = 8 + (0*5 + 1)*4 = 8 + 4 = 12
                MOV l[DS+12], 1

                ; 0 -> 2 : offset = 8 + (0*5 + 2)*4 = 8 + 8 = 16
                MOV l[DS+16], 1

                ; 1 -> 3 : offset = 8 + (1*5 + 3)*4 = 8 + 32 = 40
                MOV l[DS+40], 1

                ; 1 -> 4 : offset = 8 + (1*5 + 4)*4 = 8 + 36 = 44
                MOV l[DS+44], 1

                ; 2 -> 3 : offset = 8 + (2*5 + 3)*4 = 8 + 52 = 60
                MOV l[DS+60], 1

                ; 3 -> 4 : offset = 8 + (3*5 + 4)*4 = 8 + 76 = 84
                MOV l[DS+84], 1

                ; ---------------------------------------------------------
                ; 3. Configurar Estructura GRAFO en DS+0
                ; [GRAFO+CANT]   (DS+0) = 5 (nro de vertices)
                ; [GRAFO+MATRIZ] (DS+4) = puntero logico a la matriz (DS+8)
                ; ---------------------------------------------------------
                MOV l[DS+0], 5
                MOV EAX, DS
                ADD EAX, 8
                MOV l[DS+4], EAX

                ; ---------------------------------------------------------
                ; 4. Invocar cant_caminos(GRAFO*, DESTINO, ORIGEN)
                ; Especificacion de subrutina:
                ; PUSH <<ORIGEN>>  (0)
                ; PUSH <<DESTINO>> (4)
                ; PUSH <<GRAFO*>>  (DS+0)
                ; CALL cant_caminos
                ; ADD SP, 12
                ; ---------------------------------------------------------
                PUSH 0
                PUSH 4
                MOV EAX, DS
                ADD EAX, 0
                PUSH EAX
                CALL cant_caminos
                ADD SP, 12

                ; EAX contiene la cantidad de caminos (debe ser 3)
                RET
