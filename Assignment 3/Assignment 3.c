#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
 
int pipe_count=0, fd;
static char* args[512];
char input_buffer[1024];
char *cmd_exec[100];
int flag, len;
char cwd[1024];
int flag_pipe=1;
pid_t pid;
int flag_pipe, flag_without_pipe,  output_redirection, input_redirection;
int pid, status;
char current_directory[1000];
char *input_redirection_file;
char *output_redirection_file;

static int command(int, int, int, char *cmd_exec);

/*************************************************************************/

void clear_variables()
{
  fd =0;
  flag=0;
  len=0;
  pipe_count=0;
  flag_pipe=0;
  flag_without_pipe=0;
  output_redirection=0;
  input_redirection=0;
  input_buffer[0]='\0';
  cwd[0] = '\0';
  pid=0;

}
 
void change_directory()
{
char *h="/home";   
if(args[1]==NULL)
        chdir(h);
else if ((strcmp(args[1], "~")==0) || (strcmp(args[1], "~/")==0))
        chdir(h);
else if(chdir(args[1])<0)
    printf("error: No such file or directory\n");

}

static char* skipwhite(char* s)
{
  while (isspace(*s)) ++s;
  return s;
}

void tokenise_commands(char *com_exec)
{
int m=1;
args[0]=strtok(com_exec," ");       
while((args[m]=strtok(NULL," "))!=NULL)
        m++;
}

void tokenise_redirect_input_output(char *cmd_exec)
{
  char *io_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  io_token[0]=strtok(new_cmd_exec1,"<");       
  while((io_token[m]=strtok(NULL,">"))!=NULL)
        m++;
  io_token[1]=skipwhite(io_token[1]);
  io_token[2]=skipwhite(io_token[2]);
  input_redirection_file=strdup(io_token[1]);
  output_redirection_file=strdup(io_token[2]);
  tokenise_commands(io_token[0]);  
}

void tokenise_redirect_input(char *cmd_exec)
{
  char *i_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  i_token[0]=strtok(new_cmd_exec1,"<");       
  while((i_token[m]=strtok(NULL,"<"))!=NULL)
        m++;
  i_token[1]=skipwhite(i_token[1]);
  input_redirection_file=strdup(i_token[1]);
  tokenise_commands(i_token[0]);
}

void tokenise_redirect_output(char *cmd_exec)
{
  char *o_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  o_token[0]=strtok(new_cmd_exec1,">");       
  while((o_token[m]=strtok(NULL,">"))!=NULL)
          m++;
  o_token[1]=skipwhite(o_token[1]);
  output_redirection_file=strdup(o_token[1]); 
  tokenise_commands(o_token[0]);   
  
}

static int split(char *cmd_exec, int input, int first, int last)
{
    char *new_cmd_exec1;  
    new_cmd_exec1=strdup(cmd_exec);
      
        int m=1;
        args[0]=strtok(cmd_exec," ");       
        while((args[m]=strtok(NULL," "))!=NULL)
              m++;
        args[m]=NULL;
        if (args[0] != NULL) 
        {

            if (strcmp(args[0], "exit") == 0) 
                exit(0);

            if(strcmp("cd",args[0]) == 0)
            {
                change_directory();
                return 1;
            }

        }
        
    return command(input, first, last, new_cmd_exec1);
}

static int command(int input, int first, int last, char *cmd_exec)
{
  int mypipefd[2], ret, input_fd, output_fd;
  ret = pipe(mypipefd);
  if(ret == -1)
  {
    perror("pipe");
    return 1;
  }
  pid = fork();
 
  if (pid == 0) 
  {
    if (first==1 && last==0 && input==0) 
    {
      dup2( mypipefd[1], 1 );
    } 
    else if (first==0 && last==0 && input!=0) 
    {
      dup2(input, 0);
      dup2(mypipefd[1], 1);
    } 
    else 
    {
      dup2(input, 0);
    }
    if (strchr(cmd_exec, '<') && strchr(cmd_exec, '>')) 
        {
          input_redirection=1;
          output_redirection=1;
          tokenise_redirect_input_output(cmd_exec);
        }
   else if (strchr(cmd_exec, '<')) 
        {
          input_redirection=1;
          tokenise_redirect_input(cmd_exec);
        }
   else if (strchr(cmd_exec, '>')) 
        {
          output_redirection=1;
          tokenise_redirect_output(cmd_exec);
        }
    if(output_redirection == 1)
    {                    
        output_fd= creat(output_redirection_file, 0644);
        if (output_fd < 0)
          {
          fprintf(stderr, "Failed to open %s for writing\n", output_redirection_file);
          return(EXIT_FAILURE);
          }
        dup2(output_fd, 1);
        close(output_fd);
        output_redirection=0;
    }
    if(input_redirection  == 1)
      {
         input_fd=open(input_redirection_file,O_RDONLY, 0);
         if (input_fd < 0)
          {
          fprintf(stderr, "Failed to open %s for reading\n", input_redirection_file);
          return(EXIT_FAILURE);
          }
        dup2(input_fd, 0);
        close(input_fd);
        input_redirection=0;
      }
 
    else if(execvp(args[0], args)<0) printf("%s: command not found\n", args[0]);
              exit(0);
  }
  else 
  {
     waitpid(pid, 0, 0);  
  }
 
  if (last == 1)
    close(mypipefd[0]);

  if (input != 0) 
    close(input);

  close(mypipefd[1]);
  return mypipefd[0];

}

void with_pipe_execute()
{

	int i, n = 1, input, first;

	input = 0;
	first = 1;

	cmd_exec[0]=strtok(input_buffer,"|");

	while ((cmd_exec[n]=strtok(NULL,"|"))!=NULL)
	      n++;
	cmd_exec[n]=NULL;
	pipe_count=n-1;

	for(i = 0; i < n-1; i++)
	    {
	      input = split(cmd_exec[i], input, first, 0);
	      first=0;
	    }

	input=split(cmd_exec[i], input, first, 1);
	input=0;
	return;

}


void prompt()
{
   char shell[1000];
   char* username = getenv("USER"); 
   if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      strcpy(shell, username);
      strcat(shell, "@");
      strcat(shell, cwd);
      strcat(shell, "$ ");

      printf("%s", shell);
    }
   else
       perror("getcwd() error");

}

int main()
{   
    int status;
    char ch[2]={"\n"};
    getcwd(current_directory, sizeof(current_directory));
 
    while (1)
    {
      clear_variables();
      prompt();
      fgets(input_buffer, 1024, stdin);
      
      if(strcmp(input_buffer, ch)==0)
      {
        continue;
      }
 
      len = strlen(input_buffer);
      input_buffer[len-1]='\0';

      if(strcmp(input_buffer, "exit") == 0) 
      {
        flag = 1;
        break;
      }

      with_pipe_execute();
      waitpid(pid, &status, 0);
         
    }
  
    if(flag==1)
    {
        printf("ByeBye!\n");
        exit(0);       
        return 0;
    }

    return 0;
}
