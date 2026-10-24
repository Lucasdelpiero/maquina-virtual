NULL EQU -1

; ============================

SIZE EQU 8 ; Tamaño del nodo completo
ARBOL EQU 0 ; offset del arbol
SIG EQU 4 ; offset del siguiente

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
; Asi que EBX y Edx aunque uno es doble y el otro simple, "apuntan a lo mismo" osea [EBX] == EDX
; Y se terminan desplazando ambos, ignorando la anterior cabeza completamente.
; La ventaja de esto, es que [[EBX]] accede al nodo actual y [EBX] accede al puntero al actual (puntero del nodo anterior al actual)

lista_fin:      POP EEX
                POP EDX
                POP EBX
                MOV SP, BP
                POP BP
                RET

; ========================================================

A_SIZE EQU 8 ; Tamaño del nodo completo
A_VAL EQU 0 ; offset del valor
A_HIJOS EQU 4 ; offset del puntero a los hijos

H_SIZE EQU 8
H_ARBOL EQU 0
H_SIG EQU 4

; Especificacion de subrutina
; PUSH <<VALOR>>
; PUSH <<ROOT**>>
; CALL rec_arbol
; ADD SP, 8
; Recorre el arbol pasado por parametro, eliminando nodos que su valor sea el valor pasado por parametro
; Acumula la cantidad de eliminados en EAX      

rec_arbol:      PUSH BP
                MOV BP, SP

                PUSH EBX
                PUSH EDX ; auxliar para setear puntero (parametro) en null
                PUSH EEX ; auxiliar (puntero a puntero de los hijos del nodo arbol)
                PUSH ECX ; auxiliar (nodo arbol del hijo)

                ; Obtener puntero al arbol
                MOV EBX, [BP+8]
                MOV EBX, [EBX]; Aqui EBX == ROOT*
                CMP EBX, NULL
                JZ rec_fin

                ; Llamar a los hijos primero que nada

                PUSH [BP+12]
                MOV ECX, EBX
                ADD ECX, A_HIJOS ; Porque no olvidar que recorre_lista espera un puntero doble
                PUSH ECX
                CALL recorre_lista
                ADD SP, 8

                CMP [EBX+A_VAL], [BP+12]
                JZ rec_elim
                JMP rec_fin

rec_elim:       ADD EAX, 1
                CMP [EBX+A_HIJOS], NULL
                JZ rec_elim_simple
                MOV ECX, [EBX+A_HIJOS] ; puntero a hijos del actual - H1
                MOV EEX, [ECX+H_ARBOL] ; puntero al nodo arbol del primer hijo (el que se debe promover) - H1.nodo
                MOV ECX, [ECX+H_SIG] ; puntero al siguiente del primer hijo (se debe concatenar al ultimo hijo del que se promueve) - H1.sig

                MOV EDX, [BP+8]
                MOV [EDX], EEX ; ROOT* = hijo.arbol (nodo arbol del primer hijo, es decir el que se promueve)

                ; Recorrer los hijos en ciclo hasta llegar a NULL

                ADD EEX, A_HIJOS ; Puntero a puntero a los hijos del que se debe promover
                
rec_ciclar:     CMP [EEX], NULL
                JZ insertar
                MOV EEX, [EEX]
                ADD EEX, H_SIG
                JMP rec_ciclar

insertar:       MOV [EEX], ECX
                JMP rec_fin

rec_elim_simple: MOV EDX, [BP+8]
                MOV [EDX], NULL ; ROOT* == NULL

rec_fin:        POP ECX
                POP EEX
                POP EDX
                POP EBX
                MOV SP, BP
                POP BP
                RET
