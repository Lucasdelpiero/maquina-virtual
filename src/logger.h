#ifndef LOGGER_H
#define LOGGER_H

// Habilita o deshabilita la salida de mensajes de depuracion (1 = activo, 0 = inactivo)
void logger_habilitar(int habilitar);

// Imprime mensajes formateados solo si el logger esta habilitado
void logger(char formato[], ...);

#endif // LOGGER_H
