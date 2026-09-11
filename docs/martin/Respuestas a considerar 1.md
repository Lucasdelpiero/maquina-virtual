Leí los cuatro documentos. Voy directo a resolver lo que ya se puede decidir con la especificación y a marcar lo que sigue siendo una decisión real de ustedes.



Resuelto directamente con la especificación



\#3 — Carry en shifts (tu duda)

Sí, el shift también setea C. La especificación no distingue por operación: "Bit de acarreo (C): se activa cuando el resultado excede los 32 bits disponibles". El ejemplo de la tabla lo confirma: SHL 1073741824, 2 → resultado 0, con C=1, V=1. O sea, si al correr bits hacia la izquierda se "cae" un 1 fuera de los 32 bits, hay que marcar C (igual que si un 1 se pierde por el borde derecho en SHR/SAR). No es una suma, pero el criterio de "se perdió información por exceder el ancho" aplica igual.



Duda del uint (por qué no int32\_t simple)

Ya está resuelta y escrita en el propio Plan\_de\_Impl\_4.md, sección 4, "Regla general de tipos": con signo cuando el valor puede ser negativo por definición del dominio (registros); sin signo cuando es dirección, tamaño o contenido binario crudo (memoria, tabla de segmentos). No es una convención arbitraria — es evitar que un shift a la derecha de una dirección/tamaño se comporte distinto según el compilador (el corrimiento de un negativo no está garantizado por el estándar de C), y evitar interpretar 0xFFFFFFFF como -1 en comparaciones de límites.



Separar inicialización intrínseca vs. carga del binario

Ya está resuelto en el propio plan, sin que lo notaran: la sección 5.2 (Inicializador: crea el struct, arma los punteros a cada módulo) y la 5.3 (Carga del programa: lee cabecera, copia código, arma tabla de segmentos) ya son dos módulos separados. No hay que decidir nada nuevo, solo eran conscientes de que ya estaba resuelto.



Validar negativos en la cabecera

No aplica como validación aparte. Identificador y versión son valores fijos ("VMX26", 1) que se comparan por igualdad, no por rango — o coinciden o abortás. El único campo numérico real es "tamaño del código" (2 bytes, ya definido como uint16\_t/sin signo en tu diseño), así que "negativo" ni siquiera es un estado representable ahí. Lo que sí hay que validar (ya está en la sección 7, ítem 9 del plan) es que ese tamaño no exceda la memoria disponible.



Punto que necesita tu decisión real (inconsistencia detectada)



\#9 — Convención de nombres: NO es camelCase la convención estándar de C, es snake\_case (nombre\_variable, nombre\_funcion). CamelCase es más común en Java/JS/C#. Y esto no es solo teoría: el propio Plan\_de\_Impl\_4.md ya está mezclando ambas sin que se note:



snake\_case: tabla\_operaciones, obtener\_operacion, op\_mov, ip\_valido, leer\_instruccion

camelCase: getTipo, getDato, getValor, guardarRes, resolverDireccionMemoria, validarPos



P: Convención de nombres en C: el plan actual mezcla snake\_case (tabla\_operaciones, obtener\_operacion) con camelCase (getTipo, guardarRes, resolverDireccionMemoria). ¿Cuál unificamos?

R: Por ahora camelCase para todo, no queda igual del todo decidido por todo el equipo. Ahora siguiendo, hay mas cosas que tratar/definir/o cerrar ambiguedades? en todo caso quizas deberia decirme los modulos y funciones especiales que deberia definir su funcioanmiento antes de escribir codigo



Con eso cerrado (camelCase, aunque el equipo lo termine de confirmar), estos son los puntos que faltan resolver antes de escribir código. Los que puedo decidir directamente los resuelvo; dejo abierto solo lo que realmente depende de una elección de diseño del equipo.



Cerrados directamente (spec o buena práctica estándar)



RND, LDH, LDL, JMP y familia de saltos, SYS, STOP → NO tocan CC. La especificación lista taxativamente las 13 operaciones que sí afectan CC (MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, NOT). El snippet de ejemplo del plan (add() llamando siempre a actualizarCC) hay que tenerlo presente para no copiarlo en RND/JMP/SYS al implementarlos — es la única operación que sí llama, no la plantilla general.



DIV nunca setea C ni V. Matemáticamente el cociente de una división entera nunca puede exceder en magnitud al dividendo, así que no hay forma de "desbordar" 32 bits dividiendo. El ejemplo de la tabla (DIV 6,7 → C=0,V=0) lo confirma. Alcanza con siempre poner esos dos bits en 0 tras un DIV (solo N y Z varían).



Detección de overflow/carry en ADD, SUB, MUL: la forma más simple y fiable es calcular en un tipo de 64 bits (int64\_t resultado\_ancho = (int64\_t)a + (int64\_t)b), comparar si resultado\_ancho entra en el rango de int32\_t (eso da V), y para C mirar si el resultado sin signo excede 32 bits sin signo. Evita tener que razonar manualmente sobre acarreos internos bit a bit, que es justo la trampa que mencionaste vos mismo en el punto 3 del documento de dudas. Esto aplica a los 3, con la salvedad de MUL que puede necesitar int64\_t de entrada directamente para no perder el overflow al multiplicar en 32 bits antes de ensanchar.



SWAP con dos operandos de memoria: confirmado explícitamente al final de Lenguaje\_Assembler\_Parte\_1.md — está permitido, no hay que validar/prohibirlo.



JUMP centralizada — cómo resuelve el operando: no hace falta lógica especial. JZ \[8] usa getValor() genérico igual que cualquier operando (te da el valor ya resuelto sea registro, inmediato o memoria) y el salto es simplemente IP = CS + ese\_valor. La función JUMP no necesita distinguir tipos de operando, ya se resuelve solo con el módulo de operandos que ya definieron.



Necesita una decisión de diseño (no una sola manera correcta)



LDH / LDL no pueden reusar guardarRes tal cual está definido. guardarRes siempre pisa los 4 bytes completos del destino. LDH/LDL modifican solo 2 de los 4 bytes (la mitad alta o baja) y necesitan leer primero el valor actual del destino, combinar con los 2 bytes nuevos, y recién ahí escribir los 4 bytes completos (read-modify-write). Esto es una función aparte, no una llamada directa a guardarRes.



Helper compartido, para mantener coherencia con el diseño actual del guardarRes



Pensó durante 3s

Resolviendo los últimos detalles del diseño compartido.



Cerrado. Entonces el módulo de operandos queda con una función más, algo como:



c

void combinarMitad(Vmx \*vmx, uint8\_t tipo, int32\_t dato, uint16\_t mitadNueva, int cargarAlta) {

&#x20;   int32\_t actual = getValor(vmx, tipo, dato);

&#x20;   int32\_t combinado = cargarAlta

&#x20;       ? (actual \& 0x0000FFFF) | ((int32\_t)mitadNueva << 16)

&#x20;       : (actual \& 0xFFFF0000) | (uint16\_t)mitadNueva;

&#x20;   guardarRes(vmx, tipo, dato, combinado);

}



ldh() y ldl() llaman a esto pasando cargarAlta=1 o 0 respectivamente, con mitadNueva = los 16 bits bajos del segundo operando resuelto. Mantiene la firma uniforme y no toca guardarRes.



Lo único que falta para no quedar bloqueados



Makefile para tests (me lo delegaste): un target test que compila el runner de tests + todos los .c de tests\_\*.c + los módulos de la VM en un binario aparte del vmx final, y lo corre. Algo simple:



makefile

vmx: main.o inicializador.o memoria.o registros.o decodificador.o operadores.o disassembler.o

&#x09;gcc -o vmx $^



test: $(filter-out main.o, $(OBJS)) tests\_runner.o tests\_mov.o tests\_jumps.o tests\_aritmetica.o

&#x09;gcc -o test\_runner $^

&#x09;./test\_runner



clean:

&#x09;rm -f \*.o vmx test\_runner



(los nombres de .o/módulos son ilustrativos, se ajustan cuando definan los archivos reales). No hace falta nada más elaborado para este alcance.



Con esto, no queda ninguna ambigüedad bloqueante pendiente — el resto (Etapa 0 en adelante del roadmap) ya se puede empezar a codear. Antes de que arranquen: ¿querés que vuelque todo lo que cerramos hoy (carry en shifts, overflow con int64\_t, DIV sin overflow, RND/JMP/SYS/STOP no tocan CC, combinarMitad para LDH/LDL, camelCase, el Makefile) como una actualización del Plan\_de\_Impl\_4.md, o preferís dejarlo así en el chat y que lo vuelquen ustedes a mano?



Guardado. Decime si querés que arme el .md actualizado o seguimos con otra parte del diseño.

