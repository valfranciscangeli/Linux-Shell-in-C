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

        - Uso de fork() y execvp() para ejecutar comandos externos.

        - Manejo adecuado de procesos hijos y espera con wait().

    2. Builtins (P3):

        - Implementé todos los builtins requeridos: cd, exit, pwd, export, unset, history.

        - El historial, luego de hacer la P4, se almacena en un archivo que es leido al iniciar la shell para que se pueda manejar en memoria durante la ejecución.

    3. Funcionalidades Avanzadas (P4):

        - Historial pesistente: se guarda en el archivo shell-history, como sugiere el enunciado.

       
    4. Manejo de errores:

        - Intenté manejar todos los porsibles erorres que vi usando prints y perror. 
