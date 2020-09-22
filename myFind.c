/*
* Universidade Fernando Pessoa
* Sistemas Operativos
*
* myFind.c
*
* by:
* Eduardo Ribeiro (33812)
* Vitor Moreira (33953)
*/

#define _POSIX_SOURCE
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#undef _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <string.h>

#include <pthread.h>
#define SIZE 20



typedef int (*PARAM)(struct dirent * entry, char * value);
int name (struct dirent * entry, char * value);
int iname (struct dirent * entry, char * value);
int type (struct dirent * entry, char * value);
int size(struct dirent * entry, char *value);



const static struct
{
	const char *name;
	PARAM opt;
} function_map[] = {
	{"-name", name},
	{"-iname", iname},
	{"-type", type},
	{"-size", size},
};

typedef struct arg {
    PARAM opt;
    char * value;

}ARG;

typedef struct thread_data {
	pthread_t  thread_id;
    //path
	char *base_path;
    ARG args[50];
    int n_args;
}T_DATA;

void do_directories(const char* base_path, T_DATA t_data);
void string_to_lower(char *data);
void erro();
char get_type( struct stat file);


int name (struct dirent * entry, char * value) {

       // printf("Find by name: %s\n", value);
       // printf("d_name: %s\n", entry->d_name);
       ///comparaçao com '*'
        int i,j;
        int count =0;
        int len = strlen(value);

        for(i = 0; i<len; i++){
            if(value[i] == '*'){
                for(j=0; j<i; j++){
                    if(entry->d_name[j] == value[j])
                        count++;
                }

                if (count == len-1)
                    return 1;
            }
        }

    ///comparação normal
    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
        if(strcmp(entry->d_name, value) == 0){

        return 1; // return 1 if match found

        }

    }
    return 0;
}

int iname (struct dirent * entry, char * value) {

    char * word = malloc(sizeof(char) * 50);

    strcpy(word, entry->d_name);

    //printf("palavra: %s\n", entry->d_name);

    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){

        string_to_lower(word);
        string_to_lower(value);

        if(strcmp(word, value) == 0){

        //printf("Find by name: %s\n", value);
        //printf("d_name: %s\n", entry->d_name);
        return 1; // return 1 if match found

        }

    }
    return 0;
}

int type (struct dirent * entry, char * value) {
    //printf("Find by type: %s\n", value);

    char type[5];
    strcpy(type, value);
    //printf("char:%c\n", type[0]);

    struct stat file_stat;

    if (stat(entry->d_name, &file_stat) == 0){

        if(type[0] == get_type(file_stat)){
            return 1;
        }
    }
    return 0;

}

/*
int executable(struct dirent * entry, char * value){


}
*/

int size(struct dirent * entry, char *value)
{
	printf("Find by size: %s\n", value);
	//todo
	return 1; // return 1 if match found
}


int find_func_id(char *arg)
{
    int i;

	for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++)
	{
		if (!strcmp(function_map[i].name, arg) && function_map[i].opt)
		{
			return i;
		}
	}
	return -1;
}


void erro(){
	printf("Usage : find [path] [expression]\nexpression :\n-name <name>\n-type <type> (f,d,l,b,c,p,s)\n-executable\n-empty\n-anewer <file>\n");
	exit(1);
}


void string_to_lower(char *data) {
	int i;
	for (i = 0; data[i] != '\0'; i++) {
		data[i] = tolower(data[i]);
	}
}


char get_type( struct stat file){

    ///file blocked
    if(S_ISBLK(file.st_mode)){
        return 'b';
    }
    ///character special file
    if(S_ISCHR(file.st_mode)){
        return 'c';
    }
    ///directory
    if(S_ISDIR(file.st_mode)){
        return 'd';
    }
    ///fifo
    if(S_ISFIFO(file.st_mode)){
        return 'p';
    }
    ///regular file
    if(S_ISREG(file.st_mode)){
        return 'f';
    }
    ///symbolic link
    if(S_ISLNK(file.st_mode)){
        return 'l';
    }
    /*
    if(S_ISSOCK(file.st_mode)){
        return 's';
    }
    */

    return'?';
}


void do_files(const char* base_path, struct dirent * entry, T_DATA t_data, struct stat *file_stat){


    int i;
    for (i = 0; i < t_data.n_args; i++){
                //printf("caminho: %s\n", base_path);

        if(t_data.args[i].opt(entry, t_data.args[i].value) == 1){
            printf("%s\n", base_path);
        }
    }


/*
    printf("option: %s\n", t_data.args[0].opt);

	switch(t_data.args[0].opt){
		case name:
            if(name(entry, t_data.args[0].value))
                printf("name: %s\n", base_path);

        case iname:
            if(iname(entry, t_data.args[0].value))
                printf("iname: %s\n", base_path);
		break;

		case type:
            if(type(entry, t_data.args[0].value))
                printf("type: %s\n", base_path);
		case size:
            if(size(entry, t_data.args[0].value))
                printf("size: %s\n", base_path);
		break;

	}
	*/
}

void do_directories(const char* base_path, T_DATA t_data){

    DIR *dir;
    struct dirent *entry;

    struct stat file_stat;
    //printf("original base_path: %s\n", base_path);

    struct thread_data th_data_array[SIZE];
    int i=0;


    char *path = malloc(sizeof(char) * 300);

    if ((dir = opendir(base_path)) == NULL)
    perror("opendir() error");
  else
  {
    //puts("contents of root:");

    while ((entry = readdir(dir)) != NULL)
    {

        if (base_path[strlen(base_path) - 1] == '/')
            sprintf(path, "%s%s", base_path, entry->d_name);
        else
            sprintf(path, "%s/%s", base_path, entry->d_name);

        //sprintf(path, "%s%s", base_path, entry->d_name);

        if ((strcmp(entry->d_name, ".")) == 0 || (strcmp(entry->d_name, "..")) == 0)
            continue;

      //printf("%s\n", path);
      if (stat(path, &file_stat) == 0)
      {

            if (S_ISDIR(file_stat.st_mode)) {
                //printf("diretorio\n");
                do_files(path, entry, t_data ,&file_stat);
                //printf("directorie: %s\n", path);
                //pthread_create(&thread[i], NULL, &do_directories, &T_DATA.args[i]);
                do_directories(path, t_data);
            } else

            printf("ficheiro\n");
                do_files(path, entry, t_data ,&file_stat);

      }

      strcpy(path, "");
    }
    closedir(dir);
  }
}


int main (int argc, char * argv[]) {

	int i=0;

    char *base_path = malloc(sizeof(char) * 300);


	T_DATA t_data = { .base_path="", .args={NULL, ""}, .n_args=0 };

	t_data.base_path = argv[1];

    for(i=2; i<argc; i+=2){

        t_data.args[t_data.n_args].opt = function_map[find_func_id(argv[i])].opt;
        t_data.args[t_data.n_args].value = argv[i+1];
        t_data.n_args++;
	}

    /*
    t_data.args[t_data.n_args].opt = function_map[find_func_id(argv[4])].opt;
	t_data.args[t_data.n_args].value = argv[5];
	t_data.n_args++;
    */


	do_directories(t_data.base_path, t_data);
}
