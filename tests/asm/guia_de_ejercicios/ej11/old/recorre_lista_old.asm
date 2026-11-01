\\include "rec_arbol.asm"

SIZE EQU 8 ; Tamaño del nodo completo
ARBOL EQU 0 ; offset del arbol
SIG EQU 4 ; offset del siguiente
NULL EQU -1

; Especificacion de subrutina
; PUSH <<VALOR>>
; PUSH <LISTA**>
; CALL recorre_lista
; ADD SP, 8
; Recorre la lista pasada llamando recursivamente a eliminar del arbol el valor pasado por parametro
; Si el arbol queda vacio tambien se elimina el nodo de la lista
; Acumula la cantidad de eliminados en EAX

recorre_lista:  PUSH BP
                MOV BP, SP

                PUSH EBX
                PUSH EDX
                PUSH EEX

                ; Obtener puntero a cabeza de la lista
                MOV EBX, [BP+8]
                MOV EBX, [EBX] ; Aqui EBX == LISTA*

                CMP EBX, NULL
                JZ lista_fin

                ; Usare EDX como puntero actual y EBX sera el anterior
                MOV EDX, EBX ; Inicialmente son iguales
                JMP lista_call

comprobar:      CMP [EDX+ARBOL], NULL ; Con [EDX+ARBOL] obtengo un puntero al arbol
                JZ lista_elim

                ; Avanzar o finalizar en caso de que el puntero al arbol no se haga null
                MOV EBX, EDX
                MOV EDX, [EDX+SIG]

                CMP EDX, NULL ; Si el actual quedo en null es porque ya no tengo mas que recorrer, voy a fin
                lista_fin

lista_call:     PUSH [BP+12]
                MOV EEX, EDX
                ADD EEX, ARBOL ; Puntero doble (puntero a puntero al arbol)
                PUSH EEX
                CALL rec_arbol
                ADD SP, 8
                JMP comprobar

lista_elim:     CMP EBX, EDX
                JZ lista_elim_cabeza
                MOV [EBX+SIG], [EDX+SIG] ; siguiente del anterior es el siguiente del actual
                MOV EDX, [EDX+SIG] ; actual es el siguiente del actual
                CMP EDX, NULL
                JZ lista_fin
                JMP lista_call

lista_elim_cabeza: MOV [BP+8], [EBX+SIG]
                   MOV EBX, [EBX+SIG]
                   CMP EBX, NULL
                   JZ lista_fin
                   JMP lista_call

lista_fin:      POP EEX
                POP EDX
                POP EBX
                MOV SP, BP
                POP BP
                RET

