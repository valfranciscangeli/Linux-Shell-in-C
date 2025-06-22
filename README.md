## Archivos

- `shell.c`: El código fuente la shell en C.
- `Makefile`: Un makefile para simplificar el proceso de compilación y ejecución del shell (tal como lo dejaron en la tarea).

## Instrucciones

Para compilar el programa y generar el ejecutable `./shell`, utiliza el siguiente comando:

```bash
make
```

Si deseas compilar y ejecutar el programa en un solo paso, puedes utilizar:

```bash
make run
```

## Desarrollo:


    1. Shell Básica (P1 y P2):

        - Implementé las funciones básicas para leer, parsear y ejecutar comandos.

        - Usé fork() y execvp() para ejecutar comandos externos. Manejo procesos hijos y espera con wait().

    2. Builtins (P3):

        - Implementé todos los builtins requeridos: cd, exit, pwd, export, unset, history.

        - Agregué clear para limpiar la terminal.

        - El historial, luego de hacer la P4, se almacena en un archivo que es leído al iniciar la shell para que se pueda manejar en memoria durante la ejecución.

    3. Funcionalidades Avanzadas (P4):

        - Historial pesistente: se guarda en el archivo shell-history, como sugiere el enunciado.

        - Reemplazo por variables de entorno: Se detecta cuando el input del usuario requiere el reemplazo por una variable de entorno, utilizando el símbolo "$".

        - Procesos en segundo plano: Se agregó la opción de ejecutar comandos en segundo plano, agregando al final el símbolo "&". Se va a imprimir en pantalla el PID y su salida se redirige a "/dev/null" para que no interfiera con los procesos que se ejecutan en primer plano. Hice eso buscando una solución a los prints mezclados, porque no me gustaba como quedaba.

    4. Manejo de errores:

        - Intenté manejar todos los posibles erorres que vi usando prints y perror. 
