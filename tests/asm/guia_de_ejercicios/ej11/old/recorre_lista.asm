\\include "rec_arbol.asm"

SIZE EQU 8 ; Tamaño del nodo completo
ARBOL EQU 0 ; offset del arbol
SIG EQU 4 ; offset del siguiente
NULL EQU -1

; Especificacion de subrutina
; PUSH <<VALOR>>
; PUSH <<LISTA**>>
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
                MOV EBX, [BP+8]; Aqui EBX == LISTA**

lista_sig:      MOV EDX, [EBX] ; EDX == LISTA*
                CMP EDX, NULL ; Si el actual quedo en null es porque ya no tengo mas que recorrer, voy a fin 
                JZ lista_fin

                ; Llamar a la funcion recursiva del arbol

                PUSH [BP+12]
                MOV EEX, EDX
                ADD EEX, ARBOL ; Puntero doble (puntero a puntero al arbol)
                PUSH EEX
                CALL rec_arbol
                ADD SP, 8

                CMP [EDX+ARBOL], NULL ; Con [EDX+ARBOL] obtengo un puntero al arbol
                JZ lista_elim

                ; Avanzar
                MOV EBX, EDX
                ADD EBX, SIG
                JMP lista_sig  

lista_elim:     MOV EEX, [EDX+SIG]    ; cabeza o del medio: es igual
                MOV [EBX], EEX
                JMP lista_sig         ; el casillero no se mueve

; Explicacion de variante:
; En un caso intermedio (eliminar alguno de la mitad por ej), EBX apunta a un campo sig del anterior
; Osea que MOV [EBX], EEX, si modifica el puntero siguiente del anterior
; En el caso inicial o de la cabeza, el nodo actual no me interesa, simplemente avanzo
; Asi que EBX y Edx aunque uno es doble y el otro simple, apuntan a lo mismo
; Y se terminan desplazando ambos, ignorando la anterior cabeza completamente.
; La ventaja de esto, es que [[EBX]] accede al nodo actual y [EBX] accede al puntero al actual (puntero del nodo anterior al actual)

lista_fin:      POP EEX
                POP EDX
                POP EBX
                MOV SP, BP
                POP BP
                RET

