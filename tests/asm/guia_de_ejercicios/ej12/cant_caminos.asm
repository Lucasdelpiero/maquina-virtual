; === Funcion de entrada ===

; 12. Contar la cantidad de caminos entre un vértice origen y uno destino de un digrafo acíclico.

; Especificacion de Subrutina:
; PUSH <<ORIGEN>>
; PUSH <<DESTINO>>
; PUSH <<GRAFO*>>
; CALL cant_caminos
; ADD SP, 12
; Devuelve en EAX la cantidad de caminos de origen a destino
; Se usa como wrapper solamente para inicializar eax en cero

\\INCLUDE "rec_mat.asm"

cant_caminos:   PUSH BP
                MOV BP, SP

                MOV EAX, 0

                CMP [BP+12], [BP+16]
                JZ cant_fin ; Si destino y origen son iguales retornar cero directamente

                PUSH [BP+16]
                PUSH [BP+12]
                PUSH [BP+8]
                CALL rec_mat
                ADD SP, 12 ; Hago un ADD de 12 por la cantidad de parametros: 4*3

cant_fin:       MOV SP, BP
                POP BP
                RET