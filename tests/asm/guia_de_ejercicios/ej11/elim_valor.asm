; === Funcion de entrada ===

; 11. Dado un valor entero: eliminar de una lista dinámica simplemente enlazada, con punteros a
; árboles n-arios, todos los nodos de cada árbol con dicho valor. Cada nodo es reemplazado por
; su primer hijo. Además debe devolver la cantidad de nodos eliminados.
; Debe tener en cuenta que si un nodo de la lista queda apuntando a un árbol vacío el mismo
; debe ser eliminado

\\INCLUDE "funciones.asm"

; Especificacion de subrutina
; PUSH <<VALOR>>
; PUSH <<LISTA**>>
; CALL elim_valor
; ADD SP, 8
; Elimina de los nodos de cada arbol que contiene el valor pasado por parametro, cada arbol es apuntando
; por un nodo de la lista pasada como parametro
; Retorna en EAX la cantidad de nodos eliminados

elim_valor:     PUSH BP
                MOV BP, SP

                ; Inicializar EAX en 0, sera el contador de eliminados, y es lo que retorno
                MOV EAX, 0
                
                PUSH [BP+12]
                PUSH [BP+8]
                CALL recorre_lista
                ADD SP, 8
                
                MOV SP, BP
                POP BP
                RET
