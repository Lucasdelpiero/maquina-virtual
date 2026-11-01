\\DATA 1024
\\STACK 1024

JMP main

\\INCLUDE "amigo_ej11.asm"

; =========================================================================
; PROGRAMA DE PRUEBA INDEPENDIENTE: SOLUCION DEL AMIGO (EJERCICIO 11)
; Carga el ejemplo oficial con valor 4 (debe dar 5 eliminados).
; Llama a ProcesaLista e inspecciona el resultado retornado en ECX.
; =========================================================================

main:           MOV BP, SP

                ; ---------------------------------------------------------
                ; 1. Construir Arbol 1 (Raiz 1 en DS+64)
                ; N1_2_1 (offset 120): val = 1, hijos = NULL
                ; C1_2_1 (offset 112): arbol = N1_2_1, sig = NULL
                ; N1_2   (offset 96):  val = 2, hijos = C1_2_1
                ; N1_3_1 (offset 160): val = 4, hijos = NULL (Elimina 4 #1)
                ; N1_3_2 (offset 168): val = 5, hijos = NULL
                ; C1_3_2 (offset 152): arbol = N1_3_2, sig = NULL
                ; C1_3_1 (offset 144): arbol = N1_3_1, sig = C1_3_2
                ; N1_3   (offset 128): val = 3, hijos = C1_3_1
                ; C1_2   (offset 88):  arbol = N1_3,   sig = NULL
                ; C1_1   (offset 80):  arbol = N1_2,   sig = C1_2
                ; N1     (offset 64):  val = 1, hijos = C1_1
                ; ---------------------------------------------------------

                ; N1_2_1: val = 1, hijos = NULL
                MOV l[DS+120], 1
                MOV l[DS+124], -1

                ; C1_2_1: arbol = N1_2_1, sig = NULL
                MOV EAX, DS
                ADD EAX, 120
                MOV l[DS+112], EAX
                MOV l[DS+116], -1

                ; N1_2: val = 2, hijos = C1_2_1
                MOV l[DS+96], 2
                MOV EAX, DS
                ADD EAX, 112
                MOV l[DS+100], EAX

                ; N1_3_1: val = 4, hijos = NULL
                MOV l[DS+160], 4
                MOV l[DS+164], -1

                ; N1_3_2: val = 5, hijos = NULL
                MOV l[DS+168], 5
                MOV l[DS+172], -1

                ; C1_3_2: arbol = N1_3_2, sig = NULL
                MOV EAX, DS
                ADD EAX, 168
                MOV l[DS+152], EAX
                MOV l[DS+156], -1

                ; C1_3_1: arbol = N1_3_1, sig = C1_3_2
                MOV EAX, DS
                ADD EAX, 160
                MOV l[DS+144], EAX
                MOV EAX, DS
                ADD EAX, 152
                MOV l[DS+148], EAX

                ; N1_3: val = 3, hijos = C1_3_1
                MOV l[DS+128], 3
                MOV EAX, DS
                ADD EAX, 144
                MOV l[DS+132], EAX

                ; C1_2: arbol = N1_3, sig = NULL
                MOV EAX, DS
                ADD EAX, 128
                MOV l[DS+88], EAX
                MOV l[DS+92], -1

                ; C1_1: arbol = N1_2, sig = C1_2
                MOV EAX, DS
                ADD EAX, 96
                MOV l[DS+80], EAX
                MOV EAX, DS
                ADD EAX, 88
                MOV l[DS+84], EAX

                ; N1: val = 1, hijos = C1_1
                MOV l[DS+64], 1
                MOV EAX, DS
                ADD EAX, 80
                MOV l[DS+68], EAX

                ; ---------------------------------------------------------
                ; 2. Construir Arbol 2 (Raiz 5 en DS+180)
                ; N2_2_1 (offset 240): val = 1, hijos = NULL
                ; C2_2_1 (offset 232): arbol = N2_2_1, sig = NULL
                ; N2_2   (offset 220): val = 4, hijos = C2_2_1 (Elimina 4 #2)
                ; N2_1   (offset 200): val = 2, hijos = NULL
                ; C2_2   (offset 212): arbol = N2_2,   sig = NULL
                ; C2_1   (offset 192): arbol = N2_1,   sig = C2_2
                ; N2     (offset 180): val = 5, hijos = C2_1
                ; ---------------------------------------------------------

                ; N2_2_1: val = 1, hijos = NULL
                MOV l[DS+240], 1
                MOV l[DS+244], -1

                ; C2_2_1: arbol = N2_2_1, sig = NULL
                MOV EAX, DS
                ADD EAX, 240
                MOV l[DS+232], EAX
                MOV l[DS+236], -1

                ; N2_2: val = 4, hijos = C2_2_1
                MOV l[DS+220], 4
                MOV EAX, DS
                ADD EAX, 232
                MOV l[DS+224], EAX

                ; N2_1: val = 2, hijos = NULL
                MOV l[DS+200], 2
                MOV l[DS+204], -1

                ; C2_2: arbol = N2_2, sig = NULL
                MOV EAX, DS
                ADD EAX, 220
                MOV l[DS+212], EAX
                MOV l[DS+216], -1

                ; C2_1: arbol = N2_1, sig = C2_2
                MOV EAX, DS
                ADD EAX, 200
                MOV l[DS+192], EAX
                MOV EAX, DS
                ADD EAX, 212
                MOV l[DS+196], EAX

                ; N2: val = 5, hijos = C2_1
                MOV l[DS+180], 5
                MOV EAX, DS
                ADD EAX, 192
                MOV l[DS+184], EAX

                ; ---------------------------------------------------------
                ; 3. Construir Arbol 3 (Raiz 4 en DS+260) (Elimina 4 #3)
                ; N3: val = 4, hijos = NULL
                ; ---------------------------------------------------------
                MOV l[DS+260], 4
                MOV l[DS+264], -1

                ; ---------------------------------------------------------
                ; 4. Construir Arbol 4 (Raiz 2 en DS+280)
                ; N4_2_1 (offset 360): val = 1, hijos = NULL
                ; N4_2_2 (offset 368): val = 6, hijos = NULL
                ; N4_2_3 (offset 376): val = 7, hijos = NULL
                ; C4_2_3 (offset 352): arbol = N4_2_3, sig = NULL
                ; C4_2_2 (offset 344): arbol = N4_2_2, sig = C4_2_3
                ; C4_2_1 (offset 336): arbol = N4_2_1, sig = C4_2_2
                ; N4_2   (offset 328): val = 4, hijos = C4_2_1 (Elimina 4 #4)
                ; N4_3   (offset 384): val = 4, hijos = NULL   (Elimina 4 #5)
                ; N4_1   (offset 312): val = 2, hijos = NULL
                ; C4_3   (offset 320): arbol = N4_3,   sig = NULL
                ; C4_2   (offset 304): arbol = N4_2,   sig = C4_3
                ; C4_1   (offset 296): arbol = N4_1,   sig = C4_2
                ; N4     (offset 280): val = 2, hijos = C4_1
                ; ---------------------------------------------------------

                ; N4_2_1: val = 1, hijos = NULL
                MOV l[DS+360], 1
                MOV l[DS+364], -1

                ; N4_2_2: val = 6, hijos = NULL
                MOV l[DS+368], 6
                MOV l[DS+372], -1

                ; N4_2_3: val = 7, hijos = NULL
                MOV l[DS+376], 7
                MOV l[DS+380], -1

                ; C4_2_3: arbol = N4_2_3, sig = NULL
                MOV EAX, DS
                ADD EAX, 376
                MOV l[DS+352], EAX
                MOV l[DS+356], -1

                ; C4_2_2: arbol = N4_2_2, sig = C4_2_3
                MOV EAX, DS
                ADD EAX, 368
                MOV l[DS+344], EAX
                MOV EAX, DS
                ADD EAX, 352
                MOV l[DS+348], EAX

                ; C4_2_1: arbol = N4_2_1, sig = C4_2_2
                MOV EAX, DS
                ADD EAX, 360
                MOV l[DS+336], EAX
                MOV EAX, DS
                ADD EAX, 344
                MOV l[DS+340], EAX

                ; N4_2: val = 4, hijos = C4_2_1
                MOV l[DS+328], 4
                MOV EAX, DS
                ADD EAX, 336
                MOV l[DS+332], EAX

                ; N4_3: val = 4, hijos = NULL
                MOV l[DS+384], 4
                MOV l[DS+388], -1

                ; N4_1: val = 2, hijos = NULL
                MOV l[DS+312], 2
                MOV l[DS+316], -1

                ; C4_3: arbol = N4_3, sig = NULL
                MOV EAX, DS
                ADD EAX, 384
                MOV l[DS+320], EAX
                MOV l[DS+324], -1

                ; C4_2: arbol = N4_2, sig = C4_3
                MOV EAX, DS
                ADD EAX, 328
                MOV l[DS+304], EAX
                MOV EAX, DS
                ADD EAX, 320
                MOV l[DS+308], EAX

                ; C4_1: arbol = N4_1, sig = C4_2
                MOV EAX, DS
                ADD EAX, 312
                MOV l[DS+296], EAX
                MOV EAX, DS
                ADD EAX, 304
                MOV l[DS+300], EAX

                ; N4: val = 2, hijos = C4_1
                MOV l[DS+280], 2
                MOV EAX, DS
                ADD EAX, 296
                MOV l[DS+284], EAX

                ; ---------------------------------------------------------
                ; 5. Construir Lista Enlazada Principal (L1 -> L2 -> L3 -> L4 -> NULL)
                ; L4 (offset 40): arbol = N4 (offset 280), sig = NULL
                ; L3 (offset 32): arbol = N3 (offset 260), sig = L4 (offset 40)
                ; L2 (offset 24): arbol = N2 (offset 180), sig = L3 (offset 32)
                ; L1 (offset 16): arbol = N1 (offset 64),  sig = L2 (offset 24)
                ; lista_ptr (offset 0): apunta a L1 (offset 16)
                ; ---------------------------------------------------------

                ; L4: arbol = N4, sig = NULL
                MOV EAX, DS
                ADD EAX, 280
                MOV l[DS+40], EAX
                MOV l[DS+44], -1

                ; L3: arbol = N3, sig = L4
                MOV EAX, DS
                ADD EAX, 260
                MOV l[DS+32], EAX
                MOV EAX, DS
                ADD EAX, 40
                MOV l[DS+36], EAX

                ; L2: arbol = N2, sig = L3
                MOV EAX, DS
                ADD EAX, 180
                MOV l[DS+24], EAX
                MOV EAX, DS
                ADD EAX, 32
                MOV l[DS+28], EAX

                ; L1: arbol = N1, sig = L2
                MOV EAX, DS
                ADD EAX, 64
                MOV l[DS+16], EAX
                MOV EAX, DS
                ADD EAX, 24
                MOV l[DS+20], EAX

                ; lista_ptr (offset 0): puntero doble a L1
                MOV EAX, DS
                ADD EAX, 16
                MOV l[DS+0], EAX

                ; ---------------------------------------------------------
                ; 6. Leer por teclado el valor a eliminar (SYS 1)
                ; Guarda el valor ingresado en DS+4
                ; ---------------------------------------------------------
                MOV EAX, 1          ; Modo: Decimal
                MOV EDX, DS
                ADD EDX, 4          ; Direccion de destino para la lectura
                LDL ECX, 1          ; Cantidad = 1
                LDH ECX, 4          ; Tamano = 4 bytes
                SYS 1               ; Lee un entero decimal desde teclado

                ; ---------------------------------------------------------
                ; 7. Invocar ProcesaLista(LISTA**, VALOR)
                ; Segun especificacion del autor:
                ;   push <valor>
                ;   push <L**>
                ;   call ProcesaLista
                ;   add sp, 8
                ;   ecx = cantidad de nodos eliminados
                ; ---------------------------------------------------------
                PUSH l[DS+4]
                MOV EAX, DS
                ADD EAX, 0
                PUSH EAX
                CALL ProcesaLista
                ADD SP, 8

                ; ---------------------------------------------------------
                ; 8. Mostrar resultado por pantalla (SYS 2)
                ; Guarda el resultado (cantidad eliminada) en DS+4 y lo imprime
                ; ---------------------------------------------------------
                MOV l[DS+4], ECX

                MOV EAX, 1          ; Modo: Decimal
                MOV EDX, DS
                ADD EDX, 4          ; Direccion del valor a imprimir
                LDL ECX, 1          ; Cantidad = 1
                LDH ECX, 4          ; Tamano = 4 bytes
                SYS 2               ; Escribe valor por consola

                ; Restaurar valor de retorno en EAX y ECX
                MOV EAX, l[DS+4]
                MOV ECX, l[DS+4]
                RET
