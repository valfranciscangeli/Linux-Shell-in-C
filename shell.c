#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define MAXSIZE 1024
#define HISTORY_SIZE 10
char *history_list[HISTORY_SIZE];
int history_count = 0;

// MAcro para limpiar la pantalla
#define clear_screen() printf("\033[H\033[J")

// Muestra un prompt al usuario
void show_prompt()
{
  printf("best-shell-ever>");
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
    args[i++] = token;
    token = strtok_r(NULL, " ", &saveptr);
  }
  args[i] = NULL;
  return args;
}

// Ejecuta el comando ingresado por el usuario
void execute_command(char **cmd)
{
  pid_t pid = fork(); // crear un nuevo proceso
  if (pid == -1)
  {
    printf("\nSe produjo un error al crear el proceso hijo.\n");
    return;
  }
  else if (pid == 0)
  {
    if (execvp(cmd[0], cmd) < 0)
    {
      printf("\nSe produjo un error al ejecutar el comando: %s\n", cmd[0]);
    }
    _exit(1); // terminar el proceso hijo si execvp falla
  }
  else
  {
    // esperar que el proceso hijo termine
    wait(NULL);
    return;
  }
}

// Funcion para hacer el built-in cd
void cd(char **path)
{
  if (path[1] == NULL) // si el path es vacio, error
  {
    printf("Error: se requiere un argumento para el comando cd.\n");
  }
  else if (chdir(path[1]) != 0) // Cambiar directorio
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
void export(char **cmd)
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

void unset(char **cmd)
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
  FILE *f = fopen(getenv("HOME") ? strcat(strdup(getenv("HOME")), "/.shell-history") : ".shell-history", "a");
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
void exec_builtins(char **cmd)
{
  // CLEAR
  if (strcmp(cmd[0], "clear") == 0)
  {
    clear_screen();
  }
  // CD
  else if (strcmp(cmd[0], "cd") == 0)
  {
    cd(cmd);
  }
  // EXIT
  else if (strcmp(cmd[0], "exit") == 0)
  {
    exit(0);
  }
  // PWD
  else if (strcmp(cmd[0], "pwd") == 0)
  {
    pwd();
  }
  // EXPORT
  else if (strcmp(cmd[0], "export") == 0)
  {
    export(cmd);
  }
  // UNSET
  else if (strcmp(cmd[0], "unset") == 0)
  {
    unset(cmd);
  }
  // HISTORY
  else if (strcmp(cmd[0], "history") == 0)
  {
    history();
  }
  // si no es built-in, ejecuta el comando (ahora está de más pero lo dejo por si acaso)
  else
  {
    execute_command(cmd);
  }
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
    if (buf == NULL || buf[0] == '\0') // Maneja EOF o línea vacía
      break;
    char **cmd = parse_command(buf);
    if (cmd[0] == NULL) // No hay comando
      continue;

    // agregar el comando al historial
    add_to_history(buf);

    // revisamos si es built-in
    if (strcmp(cmd[0], "clear") == 0 ||
        strcmp(cmd[0], "cd") == 0 ||
        strcmp(cmd[0], "exit") == 0 ||
        strcmp(cmd[0], "pwd") == 0 ||
        strcmp(cmd[0], "export") == 0 ||
        strcmp(cmd[0], "unset") == 0 ||
        strcmp(cmd[0], "history") == 0)
    {
      // ejecutar comandos built-in
      exec_builtins(cmd);
    }
    else
    {
      // enviamos a ejecutar cualquier otro comando
      execute_command(cmd);
    }
  }
  return 0;
}