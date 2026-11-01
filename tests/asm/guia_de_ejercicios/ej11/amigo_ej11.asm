; =========================================================================
; CODIGO DEL AMIGO - EJERCICIO 11
; Subrutinas: ProcesaLista y ProcesaArbol
;
; NOTA: Se preserva la logica original del autor sin corregir errores
; conceptuales. Solo se adecuo la sintaxis para el ensamblador vmt.exe:
;   - Etiquetas en la misma linea que la primera instruccion.
;   - Eliminacion de espacios en expresiones de corchetes [reg+offset].
; =========================================================================

; ---- Constantes (ajustar al enunciado) ----
arbol   EQU 0      ; celda: puntero al nodo arbol
sig     EQU 4      ; celda: puntero a la celda siguiente
val     EQU 0      ; nodo: valor
lista   EQU 4      ; nodo: puntero a la primera celda de hijos
null    EQU -1

; Uso:
;   push <valor>
;   push <L**>        ; direccion del puntero a la primera celda
;   call ProcesaLista
;   add sp, 8
;   ecx = cantidad de nodos eliminados

ProcesaLista:   push bp
                mov bp, sp
                push edx
                push ebx
                push efx
                push eex
                mov edx, [bp+8]          ; edx = L**
                mov ecx, 0               ; a la ida en cero, se acumula a la vuelta
LOOPLISTA:      mov ebx, [edx]           ; celda actual
                cmp ebx, null
                jz EndLista
                add ebx, arbol           ; ebx = X** (direccion del campo arbol)
                push ecx                 ; guardo el acumulado
                push [bp+12]             ; valor
                push ebx                 ; X**
                call ProcesaArbol
                add sp, 8
                pop efx                  ; acumulado guardado
                add ecx, efx             ; + lo eliminado mas abajo
                sub ebx, arbol           ; ebx = celda
                cmp eax, null
                jnz DeleteNodoArbol
                add ebx, sig
                mov edx, ebx             ; edx = &celda.sig
                jmp LOOPLISTA

DeleteNodoArbol: add ecx, 1              ; eax = X a eliminar, ebx = celda
                mov efx, [eax+lista]     ; primera celda hija de X
                cmp efx, null
                jz DeleteNodoLista
                mov eex, [efx+arbol]     ; X' = primer hijo (el que se promueve)
                mov [ebx+arbol], eex     ; la celda ahora apunta a X'
                mov eax, [efx+sig]       ; resto de los hijos de X
                cmp eax, null
                jz Avanza
                mov efx, eax
BuscaFin:       cmp [efx+sig], null
                jz Engancha
                mov efx, [efx+sig]
                jmp BuscaFin
Engancha:       mov [efx+sig], [eex+lista]  ; ultima del resto -> hijos propios de X'
                mov [eex+lista], eax        ; X' pasa a colgar de todo el resto
Avanza:         add ebx, sig
                mov edx, ebx
                jmp LOOPLISTA

DeleteNodoLista: mov eex, [ebx+sig]      ; X sin hijos: se saca la celda
                mov [edx], eex           ; la anterior apunta a la siguiente
                jmp LOOPLISTA            ; edx no avanza

EndLista:       pop eex
                pop efx
                pop ebx
                pop edx
                mov sp, bp
                pop bp
                ret

; Uso:
;   push <val>
;   push <root**>
;   call ProcesaArbol
;   add sp, 8
;   eax = nodo a eliminar (null si no corresponde)
;   ecx = cantidad eliminada en sus descendientes

ProcesaArbol:   push bp
                mov bp, sp
                push edx
                push ebx
                mov eax, null
                mov ecx, 0
                mov edx, [bp+8]
                mov ebx, [edx]           ; nodo
                cmp ebx, null
                jz EndArbol
                cmp [ebx+lista], null
                jz ComparaValor
                add ebx, lista           ; ebx = &nodo.lista (L**)
                push [bp+12]
                push ebx
                call ProcesaLista
                add sp, 8
                sub ebx, lista
ComparaValor:   mov eax, null            ; descarta el eax que dejo ProcesaLista
                cmp [ebx+val], [bp+12]
                jnz EndArbol
                mov eax, ebx
EndArbol:       pop ebx
                pop edx
                mov sp, bp
                pop bp
                ret
