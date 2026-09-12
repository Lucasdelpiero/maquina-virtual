#ifndef ERRORES_H
#define ERRORES_H

typedef void (*AbortarHandler)(char mensaje[]);

// Muestra el mensaje de error por la salida de error (stderr) y aborta el programa inmediatamente
void abortar(char mensaje[]);

// Permite redirigir temporalmente la accion de abortar (util para testing unitario)
void set_abortar_handler(AbortarHandler handler);

#endif // ERRORES_H
