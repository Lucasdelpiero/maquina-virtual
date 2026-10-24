; Prueba completa Parte 2: Modificadores combinados de registros y memoria
\\DATA 128
\\STACK 128

; 1. Operacion sobre sector de registro EL (byte bajo de EEX)
MOV EEX, 0x1234560F
SHL EL, 1 ; EL pasa de 0x0F a 0x1E (30), sin tocar los 24 bits superiores

; 2. Copia de AH (segundo byte bajo de EAX) a celda de memoria de 1 byte b[DS+0]
MOV EAX, 0x00004A00
MOV b[DS+0], AH ; Escribe 0x4A en celda 0 de DS

; 3. Copia con direccionamiento indirecto y modificador de 2 bytes (word)
MOV EDX, DS
MOV w[EDX+4], 0x1234
MOV CX, w[EDX+4] ; CX recibe 0x1234

; 4. Transferencia de memoria a memoria entre distintos tamanos
MOV l[DS+10], 0x0000ABCD
MOV w[DS+2], l[DS+10] ; Copia los 2 bytes menos significativos (0xABCD)

MOV EAX, l[DS+0]
RET
