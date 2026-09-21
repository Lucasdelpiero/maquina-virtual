```c
#define TAM_MEMORIA_PRINCIPAL 16384 // Van a ser 16KiB
#define TAM_SEGMENTOS 8

typedef struct {
    uint32_t tabla_segmentos[TAM_SEGMENTOS];
    uint8_t mem_principal[TAM_MEMORIA_PRINCIPAL];
    uint32_t *registros;
} Memoria;
```

La memoria como TDA se encarga de la tabla de segmentos, el vector que representa la memoria principal, no se encarga de los registros, estos estan definidos en otro lugar (en la maquina virtual), pero **se tiene un puntero a ellos para poder hacer operaciones de memoria con ellos**.

Cada celda de la memoria es de 1 byte, lo que 16Kib corresponde a un vector de 16384 bytes.
Porque? en general siempre fue asi, cada celda es de 1 byte.

En el lenguaje assembler sin embargo en general leemos y escribimos de a 4 bytes, porque este es el tamaño de cada dato/variable (es decir leemos de a 4 celdas).

**La tabla de descriptores de segmetos guarda/describe donde comienza y cuando mide cada segmento**

Utilizamos la entrada 0 para representar el code segment.
Y la entrada 1 para representar el data segment.

Las dimenciones de la tabla de segmentos esta definidas en la especificacion, la tabla de segmentos debe tener 8 entradas de 32 bits.

Entonces usamos la tabla de segmentos para traducir las direcciones logicas a direcciones reales y tambien para validar accesos a memoria.

Recordar que las direcciones logicas, estan compuestas por:
Codigo de segmento en sus bytes altos: 0/1 indica el segmento que puede ser el code segment data segment
En sus bytes bajos indica el offset, que es desplazamiento desde la base del segmento dado por el codigo de segmento.

Para obtener la dirección física: Esta se obtiene como
`dir fisica = base del segmento + desplazamiento/offset`
*Mirar explicacion completa de traduccion a memoria real luego*

Donde la base del segmento se extrae de los 16 bits altos del puntero (guardado en el registro o `DS`) (1/0 en nuesto caso) , y el `offset_total` es la suma de los 16 bits bajos del registro más el desplazamiento numérico.

`[EBX+10]` ya es una direccion logica, +10 es un desplazamiento inmediato.

La maquina virtual cuando usamos un registro como puntero, carga en los 2 bytes altos la base del segmeto (1/0) y luego en los 2 bajos el valor (offset)

Usando la misma logica, esto tambien se usa para validar, el acceso, se obtiene la base, se le suma el offset y verificamos que el tamaño no exeda al tamaño del segmento.
Tambien para validar la escritura, que la memoria a escribir no supere el tamaño del segmento o que posicion base + tamaño (a escribir) <= tam maximo del segmento

Para los registros tambien se usa un vector de 32 bits o 4bytes cada componente, esta definido en la especificacion que los registros son de 32 bits.

## Como se traduce a memoria real con ejemplo

### En sintesis

1. El puntero tiene codigo del segmento (1/0) y el desplazamiento total (offset)
2. Ubicamos el segmento pedido en la tabla de segmentos, con el codigo del usuario: `entrada = mem->tabla_segmentos\[codigo del segmento];`
3. Ahora el codigo del segmento tiene por un lado la base (donde empieza en la memoria ram real) y luego el final del segmento.
4. Por ello desplazamos el tamaño y nos quedamos con la base, `base = (entrada >> 16) & 0xFFFF;` 
5. Luego aplicamos el desplazamiento a la base y obtenemos la direccion fisica:

`dir_fisica = base + offset`
`dir_fisica = 500 + 8 = 508`

Finalmente:
`dato = mem->mem_principal[508];`

### Ejemplo

La duda es completamente natural. La clave que responde tu pregunta es:

> **¿De dónde sale la `base del segmento`?**
> El programa **no sabe cuál es la base en la memoria física**. La base **está guardada únicamente adentro de la tabla de segmentos**.

---

#### Veamos un ejemplo concreto con números reales

Imaginá que tu archivo binario tiene un código que mide **500 bytes**.

Cuando la máquina virtual carga el programa en la RAM ([`Plan de Impl 5.md:L304-L312`](file:///c:/Users/defin/Desktop/FACULTAD/Facultad%202026-2C/arqui/proyecto-maquina-virtual/docs/martin/Plan%20de%20Impl%205.md#L304-L312)), configura la memoria física así:
- **Segmento de Código (entrada 0):**
  - Ocupa desde el byte físico `0` hasta el byte físico `499`.
  - **Base física = 0**.
- **Segmento de Datos (entrada 1):**
  - Comienza inmediatamente después del código: en el byte físico `500`.
  - **Base física = 500**.

Esa información se registra en la **tabla de descriptores de segmentos**:

```text
Entrada 0 (Código) -> Base Física = 0    | Tamaño = 500
Entrada 1 (Datos)  -> Base Física = 500  | Tamaño = 15884
```

---

#### Ahora el programa quiere leer un dato

El programador escribe en Assembler:
```asm
MOV EAX, [8]   ; Quiere leer la variable ubicada en el offset 8 del segmento de datos
```

La instrucción le entrega a la CPU la dirección lógica:
```text
Dirección Lógica = 0x00010008
- Código de segmento: 1  (los 16 bits altos)
- Desplazamiento: 8      (los 16 bits bajos)
```

Fijate que la dirección lógica **solo tiene el número `1` y el número `8`**. **¡No tiene el número 500 por ningún lado!**

Para saber en qué posición del vector `mem_principal[]` está realmente ese byte:

1. **La CPU toma el código de segmento `1` y va a mirar la tabla:**
   ```c
   // Va a la entrada 1 de la tabla:
   entrada = mem->tabla_segmentos[1];
   ```
2. **De esa entrada extrae la base física real en RAM:**
   ```c
   base = (entrada >> 16) & 0xFFFF;  // Obtiene 500
   ```
3. **Ahora sí puede calcular la posición física:**
   ```text
   dir_fisica = base + offset
   dir_fisica = 500 + 8 = 508
   ```
4. **Accede a la celda física del vector:**
   ```c
   dato = mem->mem_principal[508];
   ```

---

#### En resumen

Si no existiera la tabla de segmentos:
- La CPU tendría el segmento `1` y el offset `8`, pero **no tendría forma de saber en qué byte de la memoria física empieza el segmento 1** (¿empieza en el 500? ¿en el 2000? ¿en el 10000?). El tamaño del código varía para cada programa `.vmx` que ejecutes.

La tabla de segmentos es el **puente necesario** que convierte el código abstracto (`1`) en la dirección física real donde fue ubicado ese segmento en la RAM (`500`).