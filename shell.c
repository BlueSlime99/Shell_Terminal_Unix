#include<stdlib.h>
#include<stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


void affiche_cmd(const char *argv[]){
	int i=0;
	while(argv[i] != NULL){
		write(1, argv[i], strlen(argv[i]));
		i++;
	}
	
}


int parse_line(char *s, char **argv []){
	if(strpbrk(s, "=") != NULL){
		char *valeur = strpbrk(s, "=")+1;
		char *name = strtok(s, "=");
		setenv(name, valeur, 1);
	}			
	
	else if(strpbrk(s, "$") != NULL){
		if(fork() == 0){
			char *thename = getenv(strpbrk(s, "$")+1);
			//write(1, thename, strlen(thename));
			(*argv)[0] = malloc(strlen(thename));
			strcpy((*argv)[0], thename);
			(*argv)[1] = NULL;
			execvp((*argv)[0], (*argv));
		}else{
			wait(NULL);
		}
	}

	else {
		int i=0;
		int j = 0;
		int k = 0;
		while(s[i]!='\0'){
			k = 0;
			char* string = malloc(strlen(s)+1);
			while(s[i]!=' ' &&  s[i] != '\0'){
				string[k] = s[i];
				i++;
				k++;
			}
			string[k] = '\0';
			(*argv)[j] = malloc(strlen(string)+1);
			strcpy((*argv)[j], string);
			(*argv)[j][strlen(string)] = '\0';
			if(s[i] != '\0') i++;
			j++;
		}
		(*argv)[j] = NULL;
	}


	return 0;
}

void simple_cmd(char  *argv[]){
	if(argv[0] == NULL) return;
	if(!strcmp(argv[0] ,"exit")){
		exit(EXIT_SUCCESS);
		
	}
	else if(!strcmp(argv[0] ,"cd")){
		if(*(argv+1)!= NULL){
			chdir((argv[1]));
		}else{
			chdir("/");
		}
	}

	else {
		if(fork() == 0){
			execvp(argv[0], argv);
		}else{
			wait(NULL);
		}
	}
}

 void parse_line_redir( char **argv [], char **in , char **out){	
 	int i=0;
 	*in = NULL;
 	*out = NULL;

 	for(int j = 0; argv[j] != NULL; j++){
	 	while(argv[j][i]!=NULL && argv[j][i+1]!=NULL){
	 		if(!strcmp(argv[j][i], "<") && *in == NULL){
	 			*in = malloc(strlen(argv[j][i+1])+1);
	 			strcpy(*in, argv[j][i+1]);
	 			argv[j][i] = NULL;
	 		}
	 		else if(!strcmp(argv[j][i], ">") && *out == NULL){
	 			*out = malloc(strlen(argv[j][i+1])+1);
	 			strcpy(*out, argv[j][i+1]);
	 			argv[j][i] = NULL;
	 		}
	 		i++;
	 	}
	 }
 }


  int redir_cmd(char *argv[], char *in, char *out){
  	
  	if(out != NULL && in == NULL  ){
  		int outfd = open(out , O_WRONLY|O_CREAT, 00700); 
  		if(outfd == -1){
  			perror("Erreur");
  		}
  		int stdout = dup(STDOUT_FILENO);
  		dup2(outfd,STDOUT_FILENO);
  		simple_cmd(argv);
  		close(outfd);
  		dup2(stdout, STDOUT_FILENO);
  		 	
  		 
  	}else if ( in != NULL && out == NULL){
  		int infd = open( in , O_RDONLY);
  		if(infd == -1){
  			perror("Error");
  		}
  		int stdin = dup(STDIN_FILENO);
  		dup2(infd, STDIN_FILENO);
  		simple_cmd(argv);
  		close(infd);
  		dup2(stdin, STDIN_FILENO);
  			 	
  	}else if(in == NULL && out ==NULL){
  		simple_cmd(argv);
  	}
  	return 0;
  }


 void parse_line_pipes(char *s ,char *** argv[], char **in, char **out){
 	parse_line(s, *argv);
  	int i = 0;
  	int k=1;
  	while(((**argv)[i]!=NULL) || ((**argv)[i+1]!=NULL)){
  		if((**argv)[i]!=NULL && !strcmp(((**argv)[i]) ,"|")){
  			(**argv)[i] = NULL;
  			(*argv)[k] = **argv+i+1;
  			k++;
  		}
  		i++;
  	}
  	(*(*argv+k)) = NULL;
  	i=0;
  	while((*argv)[i]!= NULL){
  		parse_line_redir(*argv+i, in, out);
  		i++;
  	}
  }

 int pipes_cmd(char **argv[], char *in, char *out){
  	if(argv == NULL) return 1;
  	int buff;
  	int pipefd[2];
  	int stdout = dup(STDOUT_FILENO);
  	int stdin = dup(STDIN_FILENO);
  	int i=0;
  	while(argv[i] != NULL){
  		pipe(pipefd);
  		if(i!=0){
  			dup2(buff, STDIN_FILENO);
  			close(buff);
  		}
  		if(argv[i+1] == NULL){
  			dup2(stdout, STDOUT_FILENO);
		  	redir_cmd(argv[i], in, out);
		  	dup2(stdin, STDIN_FILENO);
		  	close(stdin);
		  	close(stdout);
		  	break;
  		}
  		dup2(pipefd[1], STDOUT_FILENO);
  		close(pipefd[1]);
  		//affiche_cmd((const char**)argv[i]);
  		simple_cmd(argv[i]);
  		buff = dup(pipefd[0]); 
  		close(pipefd[0]); 
  		i++;
  	}
  	//affiche_cmd((const char**)argv[i]);
  	/*dup2(stdout, STDOUT_FILENO);
  	redir_cmd(argv[i], in, out);
  	dup2(stdin, STDIN_FILENO);
  	close(stdin);
  	close(stdout);*/
  	return 0;
 }
void handler(int signum){
	if(signum == SIGTSTP){
		write(STDOUT_FILENO, "\n", 1);
		write(1, "Un signal SIGTSTP à été émis", strlen(" un signal SIGTSTP à été émis") );
	}
}




/*FREE CHAQUE CASES ALLOUÉES*/
void freeArg(char** arg){
	for(int i = 0; arg[i] != NULL; i++){
		free(arg[i]);
	}
	free(arg);
}


int main(int argc, char *argv[])
{	
		
		if(argv[1] == NULL ){
				struct sigaction act;
				act.sa_handler = &handler;
				act.sa_flags = SA_RESTART;
				sigemptyset(&act.sa_mask);
				sigaction(SIGTSTP, &act, NULL);
				char buffer1[1024];
				char buffer2[1024];
				char*** argv2 = malloc(sizeof(char**) * 100);
				*argv2 = malloc(sizeof(char*) * 100);
				char* exmpl1 = NULL;
				char* exmpl2 = NULL;
				while(1){
				 	getcwd(buffer1, 1024);
				 	write(STDOUT_FILENO, buffer1, strlen(buffer1));
				 	write(STDOUT_FILENO, "$ ", 2);
				 	ssize_t read_char = read(STDIN_FILENO, buffer2, 1024);
				 	buffer2[read_char-1] = '\0'; 
				 //	parse_line(buffer2, &argv2);
				 	///parse_line_redir(buffer2, &argv2, &exmpl1, &exmpl2);
				 	parse_line_pipes(buffer2 ,&argv2, &exmpl1, &exmpl2);
				 	//pipes_cmd(argv2, exmpl1, exmpl2);
				 	//write(STDOUT_FILENO, "\n", 1);
				 	pipes_cmd(argv2, exmpl1, exmpl2);
				 	//affiche_cmd((const char**)argv);
				 	

				
				}
			
		}else{
			if(argc >1){
				if(!strcmp(strpbrk(argv[1], "."), ".sh")){

					int fd = open(argv[1], O_RDONLY);
					char buffer[1024] = {0};
					char buffercpy[1024] = {0};
					char **commande = malloc(sizeof(char*)*100);
					while(read(fd, buffer, 1024) !=0){
						strcpy(buffercpy, buffer);
						char *ret, *save;
						ret = strtok_r(buffercpy, "\n", &save);
						parse_line(ret, &commande);
						simple_cmd(commande);
						freeArg(commande);
						write(1, ret, strlen(ret));
						while((ret = strtok_r(NULL, "\n", &save)) != NULL){
							char **commande = malloc(sizeof(char*)*100);
							write(1, ret, strlen(ret));
							parse_line(ret, &commande);
							simple_cmd(commande);
						    freeArg(commande);
						}

						
							//execvp(commande[0], commande);
						}
						close(fd);
					}
					
					exit(EXIT_SUCCESS);
				}
			}
     	return 0;
}