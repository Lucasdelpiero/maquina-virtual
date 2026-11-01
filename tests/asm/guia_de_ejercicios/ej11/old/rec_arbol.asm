\\include "recorre_lista.asm"

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
      
rec_arbol:      PUSH BP
                MOV BP, SP

                PUSH EBX
                PUSH EDX ; auxliar para setear puntero (parametro) en null
                PUSH EEX ; auxiliar (puntero a puntero de los hijos del nodo arbol)
                PUSH ECX ; auxiliar (nodo arbol del hijo)

                ; Obtener puntero al arbol
                MOV EBX, [BP+8]
                MOV EBX, [EBX]; Aqui EBX == ROOT*

                ; Llamar a los hijos primero que nada

                PUSH [BP+12]
                PUSH [EBX+A_HIJOS]
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

rec_elim_simple:MOV EDX, [BP-8]
                MOV [EDX], NULL ; ROOT* == NULL

rec_fin:        POP ECX
                POP EEX
                POP EDX
                POP EBX
                MOV SP, BP
                POP BP
                RET