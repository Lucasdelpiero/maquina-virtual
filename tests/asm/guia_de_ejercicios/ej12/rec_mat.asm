TAM EQU 4
CANT EQU 0
MATRIZ EQU 4

; Especificacion de Subrutina:
; PUSH <<ORIGEN>>
; PUSH <<COLUMNA ACTUAL>>
; PUSH <<GRAFO*>>
; CALL rec_mat
; ADD SP, 12
; Devuelve en EAX la cantidad de caminos de origen a destino, requiere que el llamador inicialize en cero
; Pasar columna actual como destino en el llamado de esta funcion

rec_mat:        PUSH BP
                MOV BP, SP

                PUSH EBX
                PUSH EDX
                PUSH ECX
                PUSH EEX
                PUSH EFX
                PUSH AC; Auxiliar contendra la direccion de celda actual (puntero)

                ; EDX Contendra el n
                MOV EDX, [BP+8]
                MOV EDX, [EDX+CANT]

                ; EFX Contendra el indice del tope, que es N basicamente
                MOV EFX, EDX

                ; EBX Contendra la MATRIZ, esto es en realidad un puntero a la primer celda de la matriz
                MOV EBX, [BP+8] ; GRAFO*
                MOV EBX, [EBX+MATRIZ] ; Puntero a la primer celda de la matriz

                ; ECX contendra la cantidad de memoria a desplazar para saltar entre filas:
                MOV ECX, TAM
                MUL ECX, EDX

                ; EEX Contendra la posicion de la fila actual en la columna
                ; Inicialmente se posicionara en 0 por la fila cero
                MOV EEX, 0

                MOV AC, EBX ; Hacer que AC apunte a la primer celda de la matriz, SOLO AC, EBX ya que EBX ya era un puntero
                MOV EDX, [BP+12] ; Reusar EDX como auxiliar
                MUL EDX, TAM ; la fila cero de la columna destino se calcula como offset matriz + TAM * destino
                ADD AC, EDX

                ; RECORDAR: Siempre empiezo comparando la condicion de corte de la funcion, siempre termina resultando asi

                ; Si EEX es mayor o igual al tope voy a terminar la funcion, ya no hay mas que recorrer en la columna
rec_ciclo:      CMP EEX, EFX ; Al recorrer la matriz conviene cortar por indices y no por cantidad de bytes
                JNN rec_fin ; JPZ es JNN en realidad
                
                ; Si es 0: Salto directamente
                CMP [AC], 0  ; [AC] es mat [i][j]
                JZ saltar

                ; Ya sabiendo que es 1, Verificar si coincide con el origen, en este caso solo sumar y avanzar
                CMP EEX, [BP+16]
                JZ rec_sumar

                ; Si es uno, y no concide con el origen, llamo de forma recursiva
                JMP rec_call

                ; Sumar fila y cantidades de bytes para saltar a la proxima fila
saltar:         ADD EEX, 1 ; fila += 1
                ADD AC, ECX ; saltar de fila
                JMP rec_ciclo

rec_call:       PUSH [BP+16]
                PUSH EEX ; Pushear la columna a analizar
                PUSH [BP+8] ; GRAFO
                CALL rec_mat
                ADD SP, 12
                JMP saltar
                
rec_sumar:      ADD EAX, 1
                JMP saltar

rec_fin:        POP AC
                POP EFX
                POP EEX
                POP ECX
                POP EDX
                POP EBX
                MOV SP, BP
                POP BP
                RET

; Claves a recordar de este ejercicio:
;
; 1) Recordar ir poniendo al lado cosas como estas asi me queda mas claro y no cometo errores:
;                 MOV EBX, [BP+8] ; GRAFO*
;                 MOV EBX, [EBX+MATRIZ] ; Puntero a la primer celda de la matriz
;
; 2) Para recorrer matrices, debo llevar un contador del indice y por otro lado un registro que tenga la cantidad de bytes
; equivalente, usar para comparar el registro que tiene el indice y el que lleva los bytes solo usarlo para acceder a memoria
;
; 3) EL ADD SP, despues de llamar a la funcion coincide con la cantidad de parametros * 4, no tiene nada que ver con el orden de los
; parametros en la pila