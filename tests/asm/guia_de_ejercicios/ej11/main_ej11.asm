\\DATA 1024
\\STACK 1024

JMP main

\\INCLUDE "elim_valor.asm"

; =========================================================================
; PROGRAMA PRINCIPAL DE PRUEBA: EJERCICIO 11
; Construye en DS:
; Lista enlazada con 3 arboles n-arios:
;   - Arbol 1: Raiz con valor 4 (elimina raiz y promueve 1er hijo), 1 hijo hoja con valor 4.
;   - Arbol 2: Arbol compuesto unicamente por nodos con valor 4 (raiz 4 y su unico hijo 4).
;              Al eliminarse ambos, el arbol queda vacio y se desvincula el nodo de la lista.
;   - Arbol 3: Raiz con valor 30, un hijo intermedio con valor 4 que tiene a su vez un hijo con valor 50.
; Total de nodos con valor 4 eliminados = 5.
; EAX debe terminar con valor 5.
; =========================================================================

main:           MOV BP, SP

                ; ---------------------------------------------------------
                ; 1. Construir Arbol 1
                ; Nodos Arbol:
                ; T1_1 (offset 80):  val = 10, hijos = C1_3 (offset 160)
                ; T1_2 (offset 96):  val = 4,  hijos = NULL
                ; T1_3 (offset 112): val = 20, hijos = NULL
                ; T1   (offset 64):  val = 4,  hijos = C1_1 (offset 128)
                ; Nodos Hijos de T1:
                ; C1_1 (offset 128): arbol = T1_1 (offset 80),  sig = C1_2 (offset 144)
                ; C1_2 (offset 144): arbol = T1_2 (offset 96),  sig = C1_3 (offset 160)
                ; C1_3 (offset 160): arbol = T1_3 (offset 112), sig = NULL
                ; ---------------------------------------------------------

                ; T1_1: val = 10, hijos = NULL
                MOV l[DS+80], 10
                MOV l[DS+84], -1

                ; T1_2: val = 4, hijos = NULL
                MOV l[DS+96], 4
                MOV l[DS+100], -1

                ; T1_3: val = 20, hijos = NULL
                MOV l[DS+112], 20
                MOV l[DS+116], -1

                ; C1_3: arbol = T1_3, sig = NULL
                MOV EAX, DS
                ADD EAX, 112
                MOV l[DS+160], EAX
                MOV l[DS+164], -1

                ; C1_2: arbol = T1_2, sig = C1_3
                MOV EAX, DS
                ADD EAX, 96
                MOV l[DS+144], EAX
                MOV EAX, DS
                ADD EAX, 160
                MOV l[DS+148], EAX

                ; C1_1: arbol = T1_1, sig = C1_2
                MOV EAX, DS
                ADD EAX, 80
                MOV l[DS+128], EAX
                MOV EAX, DS
                ADD EAX, 144
                MOV l[DS+132], EAX

                ; T1 (Raiz Arbol 1): val = 4, hijos = C1_1
                MOV l[DS+64], 4
                MOV EAX, DS
                ADD EAX, 128
                MOV l[DS+68], EAX

                ; ---------------------------------------------------------
                ; 2. Construir Arbol 2 (quedara vacio -> se elimina de la lista)
                ; T2_1 (offset 208): val = 4, hijos = NULL
                ; C2_1 (offset 224): arbol = T2_1, sig = NULL
                ; T2   (offset 192): val = 4, hijos = C2_1
                ; ---------------------------------------------------------

                ; T2_1: val = 4, hijos = NULL
                MOV l[DS+208], 4
                MOV l[DS+212], -1

                ; C2_1: arbol = T2_1, sig = NULL
                MOV EAX, DS
                ADD EAX, 208
                MOV l[DS+224], EAX
                MOV l[DS+228], -1

                ; T2 (Raiz Arbol 2): val = 4, hijos = C2_1
                MOV l[DS+192], 4
                MOV EAX, DS
                ADD EAX, 224
                MOV l[DS+196], EAX

                ; ---------------------------------------------------------
                ; 3. Construir Arbol 3
                ; T3_1_1 (offset 288): val = 50, hijos = NULL
                ; C3_1_1 (offset 304): arbol = T3_1_1, sig = NULL
                ; T3_1   (offset 272): val = 4,  hijos = C3_1_1 (offset 304)
                ; C3_1   (offset 256): arbol = T3_1, sig = NULL
                ; T3     (offset 240): val = 30, hijos = C3_1 (offset 256)
                ; ---------------------------------------------------------

                ; T3_1_1: val = 50, hijos = NULL
                MOV l[DS+288], 50
                MOV l[DS+292], -1

                ; C3_1_1: arbol = T3_1_1, sig = NULL
                MOV EAX, DS
                ADD EAX, 288
                MOV l[DS+304], EAX
                MOV l[DS+308], -1

                ; T3_1: val = 4, hijos = C3_1_1
                MOV l[DS+272], 4
                MOV EAX, DS
                ADD EAX, 304
                MOV l[DS+276], EAX

                ; C3_1: arbol = T3_1, sig = NULL
                MOV EAX, DS
                ADD EAX, 272
                MOV l[DS+256], EAX
                MOV l[DS+260], -1

                ; T3 (Raiz Arbol 3): val = 30, hijos = C3_1
                MOV l[DS+240], 30
                MOV EAX, DS
                ADD EAX, 256
                MOV l[DS+244], EAX

                ; ---------------------------------------------------------
                ; 4. Construir Lista Enlazada Principal (L1 -> L2 -> L3 -> NULL)
                ; L1 en offset 16: arbol = T1 (offset 64),  sig = L2 (offset 32)
                ; L2 en offset 32: arbol = T2 (offset 192), sig = L3 (offset 48)
                ; L3 en offset 48: arbol = T3 (offset 240), sig = NULL
                ; lista_ptr en offset 0: apunta a L1
                ; ---------------------------------------------------------

                ; L3: arbol = T3, sig = NULL
                MOV EAX, DS
                ADD EAX, 240
                MOV l[DS+48], EAX
                MOV l[DS+52], -1

                ; L2: arbol = T2, sig = L3
                MOV EAX, DS
                ADD EAX, 192
                MOV l[DS+32], EAX
                MOV EAX, DS
                ADD EAX, 48
                MOV l[DS+36], EAX

                ; L1: arbol = T1, sig = L2
                MOV EAX, DS
                ADD EAX, 64
                MOV l[DS+16], EAX
                MOV EAX, DS
                ADD EAX, 32
                MOV l[DS+20], EAX

                ; lista_ptr (offset 0): puntero a L1
                MOV EAX, DS
                ADD EAX, 16
                MOV l[DS+0], EAX

                ; ---------------------------------------------------------
                ; 5. Invocar elim_valor(LISTA**, VALOR)
                ; PUSH 4        (VALOR)
                ; PUSH LISTA**  (&lista_ptr en DS+0)
                ; CALL elim_valor
                ; ADD SP, 8
                ; ---------------------------------------------------------
                PUSH 4
                MOV EAX, DS
                ADD EAX, 0
                PUSH EAX
                CALL elim_valor
                ADD SP, 8

                ; EAX contiene la cantidad de nodos eliminados (debe ser 5)
                RET
