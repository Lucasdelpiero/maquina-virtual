12. Contar la cantidad de caminos entre un vértice origen y uno destino de un digrafo acíclico.

Digrafo = tiene direccion

aciclico = no tiene ciclos

El puntero al grafo contiene un puntero a la matriz de adyacencia
Y contiene la cantidad de vertices, que es basicamente el nro de filas y columnas.

En la matriz, al ser cuadrada, cada fila tiene n posiciones y cada columna tiene n posiciones
Por lo que para avanzar por columna debo sumar 4, al igual que siempre
Pero para avanzar en las filas debo sumar 4*n, idealmente guardar este calculo en un registro AC.

Entonces los parametros son:
puntero
inicio
fin

Deberia irme a la fila inicio y recorrer sola esa fila.

Algoritmo:

1) Arranco en la columna destino, debere tener un ciclo para iterar hasta llegar a destino
2) Llamo recusivamente para los que apuntan al destino.
3) Si alguno de estos es apuntado por origen, sumo 1 a eax
4) Llamo recursivamente a los que apuntan a estos ultimos, analizando sus columnas
5) El ciclo se repite
6) La recursion termina al llegar a aquellos vertices que no son apuntados por nadie.


En realidad en ningun momento recorro filas, solo recorro columnas, asi que debo moverme por filas, osea usare 4*n, usare AC como variable auxiliar.

Debere usar una funcion tonta para inicializar eax en cero, solo inicializa y llama a la funcion recursiva

