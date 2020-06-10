#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int cd(char **args);
int ls(char **args);
int exitt(char **args);

// command list
char *builtin_str[] = {
  "cd",
  "ls",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &cd,
  &ls,
  &exitt
};

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//command implemtation
int cd(char **args)
{	
  if (args[1] == NULL) {
	fprintf(stderr, "expected argument to \"cd\"\n");
  } 
	else {
		if (chdir(args[1]) != 0) {
			perror("error");
		}
  }
  return 1;
}

int exitt(char **args)
{
		printf("ByeBye!\n");
  	return 0;
}

int ls(char **args)
{
		int j = 1;
		while(args[j] != NULL)
		{
			j++;
		}

    printf("[CMD]%s", args[0]);	
    if (args[1] != NULL) {
	  printf("[ARGV]");
			for(int i = 1; i < j; i++) {
				printf("%s", args[i]);
				if (i + 1 < j)
					printf(",");
			}
    }
    
    printf("\n"); 
    return 1;
}

//Execute shell built-in or launch program.
int execute(char **args)
{
  int i;

  if (args[0] == NULL) {	
		return 1;
  }

  for (i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
  }

  return 1;
}

#define RL_BUFSIZE 1024

//Read a line of input from stdin.
char *read_line(void)
{
  int bufsize = RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
	fprintf(stderr, "allocation error\n");
	exit(EXIT_FAILURE);
  }

  while (1) {
	// Read a character
	c = getchar();

	// If we hit EOF, replace it with a null character and return.
	if (c == EOF || c == '\n') {
	  buffer[position] = '\0';
	  return buffer;
	} 
	else {
	  buffer[position] = c;
	}
	position++;

	// If we have exceeded the buffer, reallocate.
	if (position >= bufsize) {
	  bufsize += RL_BUFSIZE;
	  buffer = realloc(buffer, bufsize);
	  if (!buffer) {
	    fprintf(stderr, "allocation error\n");
	    exit(EXIT_FAILURE);
	  }
	}
  }
}

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

//Split a line into tokens.
char **split_line(char *line)
{
  int bufsize = TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
	fprintf(stderr, "allocation error\n");
	exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);//split string
  while (token != NULL) {
	tokens[position] = token;
	position++;

	if (position >= bufsize) {
	  bufsize += TOK_BUFSIZE;
	  tokens = realloc(tokens, bufsize * sizeof(char*));
	  if (!tokens) {
	    fprintf(stderr, "allocation error\n");
	    exit(EXIT_FAILURE);
	  }
	}

	token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

//Loop getting input and executing it.
void loop(void)
{
  char *line;
  char **args;
  int status;

  do {
	char* username = getenv("USER"); 
	printf("%s@", username); 

	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd));
	printf("%s", cwd); 
    
	printf("> ");
    
	line = read_line();
	args = split_line(line);
	status = execute(args);

	free(line);
	free(args);
  } while (status);
}

int main(int argc, char **argv)
{

  loop();

  return EXIT_SUCCESS;
}
