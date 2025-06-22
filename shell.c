#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

// Definiciones
#define MAXSIZE 1024
#define HISTORY_SIZE 10 // cambiar si se quiere guardar mas comandos en el historial
char *history_list[HISTORY_SIZE];
int history_count = 0;

// Declaracion de funciones
char *expand_variables(char *arg);

// Macro para limpiar la pantalla
#define clear_screen() printf("\033[H\033[J")

// ============= Funciones del shell =============

// Muestra un prompt al usuario
void show_prompt()
{
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("[%s] best-shell-ever> ", cwd);
  } else {
    printf("[?] best-shell-ever> ");
  }
  fflush(stdout); // asegurarse de que el prompt se muestre inmediatamente
}

// Lee la entrada del usuario
char *read_input()
{
  static char buf[MAXSIZE]; // static para que persista entre llamadas
  if (fgets(buf, MAXSIZE, stdin) == NULL)
    return NULL;
  buf[strcspn(buf, "\n")] = 0; // eliminar salto de línea
  return buf;
}

// Funcion para expandir variables de entorno
char *expand_variables(char *arg)
{
  if (arg[0] != '$')
    return strdup(arg); // no es variable si no empieza con $

  char *nombre = arg + 1; // omitir el $
  char *valor = getenv(nombre);

  if (valor)
    return strdup(valor); // si era variables
  else
    return strdup(""); // la variable no existe
}

// Parsea el comando ingresado por el usuario
char **parse_command(char *buf)
{
  static char *args[100]; // static para que persista entre llamadas
  char *token;
  char *saveptr;
  int i = 0;
  token = strtok_r(buf, " ", &saveptr);
  while (token != NULL)
  {
    args[i++] = expand_variables(token); // se modifica aca para revisar si se usa una variable de entorno
    if (i >= 99)
      break; // evitar que se desborde el arreglo

    token = strtok_r(NULL, " ", &saveptr);
  }
  args[i] = NULL;
  return args;
}

// Ejecuta el comando ingresado por el usuario
void execute_command(char **cmd, int is_background)
{
  pid_t pid = fork(); // crear un nuevo proceso

  if (pid == -1)
  {
    printf("\nSe produjo un error al crear el proceso hijo.\n");
    return;
  }
  else if (pid == 0)
  {
    // redirigir las salidas si es un comando en segundo plano
    if (is_background)
    {
      int devnull = open("/dev/null", O_WRONLY);
      if (devnull != -1)
      {
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO); 
        close(devnull);
      }
    }

    if (execvp(cmd[0], cmd) < 0)
    {
      printf("\nSe produjo un error al ejecutar el comando: %s\n", cmd[0]);
    }
    _exit(1); // terminar el proceso hijo si hay error
  }
  else
  {
    if (is_background) // si es un comando en segundo plano

    {
      printf("Comando '%s' en segundo plano con PID %d\n", cmd[0], pid);
      fflush(stdout);
    }
    else // si es un comando en primer plano

    {
      wait(NULL); // esperar a que el proceso hijo termine
    }
  }
}

// Funcion para hacer el built-in cd
void cd(char **path)
{
  if (path[1] == NULL) // si el path es vacio, error
  {
    printf("Error: se requiere un argumento para el comando cd.\n");
  }
  else if (chdir(path[1]) != 0) // cambiar directorio
  {
    perror("Error al cambiar de directorio");
  }
}

// Funcion para hacer el built-in pwd
void pwd()
{
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) // se intenta obtener el directorio actual
  {
    printf("%s\n", cwd);
  }
  else
  {
    perror("Error al obtener el directorio actual");
  }
}

// Funcion para hacer el built-in export
void export_variable(char **cmd)
{
  if (cmd[1] == NULL)
  {
    fprintf(stderr, "Hint: export NOMBRE=VALOR\n");
  }
  else
  {
    char *name = strtok(cmd[1], "=");
    char *val = strtok(NULL, "=");
    if (name && val)
    {
      if (setenv(name, val, 1) != 0)
        perror("Se produjo un error al exportar la variable");
    }
    else
    {
      fprintf(stderr, "Formato inválido. Hint: export NOMBRE=VALOR\n");
    }
  }
}

void unset_variable(char **cmd)
{
  if (cmd[1] == NULL)
  {
    fprintf(stderr, "Hint: unset NOMBRE\n");
  }
  else
  {
    if (unsetenv(cmd[1]) != 0)
    {
      perror("Se produjo un error al eliminar la variable de entorno");
    }
  }
}

// Funcion para agregar un comando al historial
void add_to_history(char *buf)
{
  if (history_count < HISTORY_SIZE)
  {
    history_list[history_count++] = strdup(buf);
  }
  else
  {
    free(history_list[0]); // liberar el primero
    for (int i = 1; i < HISTORY_SIZE; i++)
    {
      history_list[i - 1] = history_list[i]; // se desplazan los comandos hacia la izquierda
    }
    history_list[HISTORY_SIZE - 1] = strdup(buf); // agregar el comando nuevo al final
  }
  // guardar historial en un archivo
  char path[1024];
  snprintf(path, sizeof(path), "%s/.shell-history", getenv("HOME") ?: ".");
  FILE *f = fopen(path, "a");

  if (f)
  {
    fprintf(f, "%s\n", buf);
    fclose(f);
  }
}

// Funcion para cargar el historial desde un archivo
void load_history()
{
  char path[1024];
  snprintf(path, sizeof(path), "%s/.shell-history", getenv("HOME"));
  FILE *f = fopen(path, "r");
  if (!f)
    return;
  char line[MAXSIZE];
  while (fgets(line, sizeof(line), f) && history_count < HISTORY_SIZE)
  {
    line[strcspn(line, "\n")] = 0;
    history_list[history_count++] = strdup(line);
  }
  fclose(f);
}

void history()
{
  for (int i = 0; i < history_count; ++i)
  {
    printf("-> %s\n", history_list[i]);
  }
}

// Funcion para revisar si se quiere ejecutar un comando built-in
int exec_builtins(char **cmd)
{
  // CLEAR
  if (strcmp(cmd[0], "clear") == 0)
  {
    clear_screen();
    return 1;
  }
  // CD
  else if (strcmp(cmd[0], "cd") == 0)
  {
    cd(cmd);
    return 1;
  }
  // EXIT
  else if (strcmp(cmd[0], "exit") == 0)
  {
    exit(0);
    return 1;
  }
  // PWD
  else if (strcmp(cmd[0], "pwd") == 0)
  {
    pwd();
    return 1;
  }
  // EXPORT
  else if (strcmp(cmd[0], "export") == 0)
  {
    export_variable(cmd);
    return 1;
  }
  // UNSET
  else if (strcmp(cmd[0], "unset") == 0)
  {
    unset_variable(cmd);
    return 1;
  }
  // HISTORY
  else if (strcmp(cmd[0], "history") == 0)
  {
    history();
    return 1;
  }
  return 0;
}

// Funcion para revisar si se quiere ejecutar un comando en segundo plano. Revisa si el comando termina con '&'
int is_background_command(char **cmd)
{

  int i = 0;
  while (cmd[i] != NULL) // contar el largo del comando
    i++;

  if (i > 0 && strcmp(cmd[i - 1], "&") == 0)
  {
    cmd[i - 1] = NULL; // eliminar el '&' del comando
    return 1;          // si es en segundo plano
  }

  return 0; // no es en segundo plano
}

// función principal del shell
int main(void)
{
  load_history(); // cargar historial al iniciar
  clear_screen(); // limpiar la pantalla al iniciar
  while (1)
  {
    show_prompt();
    char *buf = read_input();

    // si falla, se sale del bucle
    if (buf == NULL) 
      break;

    // si se presiona ENTER (vacio), se ignora
    if (buf[0] == '\0')
      continue;

    char **cmd = parse_command(buf);
    if (cmd[0] == NULL) // No hay comando
      continue;

    int is_background = is_background_command(cmd);

    // agregar el comando al historial
    add_to_history(buf);

    // si no era built-in, se ejecuta normalmente
    if (!exec_builtins(cmd))
    {
      execute_command(cmd, is_background);
    }
  }
  return 0;
}