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
#define SIZE 100



typedef int (*PARAM)(struct dirent * entry, char * value);
int name (struct dirent * entry, char * value);
int iname (struct dirent * entry, char * value);
int type (struct dirent * entry, char * value);
int size(struct dirent * entry, char *value);
int mmin(struct dirent * entry, char *value);
int executable(struct dirent * entry, char *value);



const static struct
{
	const char *name;
	PARAM opt;
} function_map[] = {
	{"-name", name},
	{"-iname", iname},
	{"-type", type},
	{"-size", size},
	{"-mmin", mmin},
	{"-executable", executable},
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
    int n_thread;
}T_DATA;

void* do_directories(void* threadarg);
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

        ///nao entra?
        if(type[0] == get_type(file_stat)){
            return 1;
        }
    }
    return 0;

}

int mmin (struct dirent * entry, char * value) {
    printf("Find by mmin: %s\n", value);

    struct stat file_stat;
    time_t aux;

    aux = time(NULL) + (60+atoi(value));

    /*
    if (stat(entry->d_name, &file_stat) == 0){
        if(file_stat.ntime > aux){
            return 1;
        ///nao entra?
        printf("ola\n");

        }
    }
    */
    return 0;

}


int executable(struct dirent * entry, char * value){
    printf("Find by executable:\n");

    struct stat file_stat;

    if(file_stat.st_mode & S_IXUSR)
        return 1;

    return 0;
}


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

//void do_directories(const char* base_path, T_DATA t_data);

void* do_directories(void* threadarg){

    DIR *dir;
    struct dirent *entry;

    struct stat file_stat;
    //printf("original base_path: %s\n", base_path);

    char *path = malloc(sizeof(char) * 300);

        int i=0;
        int j;
        int k;
	//cast args
    T_DATA *my_data;
    my_data = (T_DATA*) threadarg;

	T_DATA th_data_array[SIZE];

   // my_data->n_thread++;
    //int k = my_data->n_thread;
    //int i=k;

    //printf("\n");
    //printf("path recebido: %s, thread_id:%lu, n_thread:%d\n", my_data->base_path,(unsigned long) my_data->thread_id, i);
        /*
        th_data_array[i].args[0].opt = my_data->args[0].opt;
        th_data_array[i].args[0].value = my_data->args[0].value;
        //th_data_array[i].n_thread = i;
        */

        //printf("opt: %s , value:%s: \n", th_data_array[k].args[0].opt, th_data_array[k].args[0].value);

    if ((dir = opendir(my_data->base_path)) == NULL)
    perror("opendir() error");
  else
  {
    //puts("contents of root:");

    while ((entry = readdir(dir)) != NULL)
    {

        if (my_data->base_path[strlen(my_data->base_path) - 1] == '/')
            sprintf(path, "%s%s", my_data->base_path, entry->d_name);
        else
            sprintf(path, "%s/%s", my_data->base_path, entry->d_name);

            //printf("paht: %s\n", path);
        //sprintf(path, "%s%s", base_path, entry->d_name);

        if ((strcmp(entry->d_name, ".")) == 0 || (strcmp(entry->d_name, "..")) == 0)
            continue;

      //printf("%s\n", path);
      if (stat(path, &file_stat) == 0)
      {

            //printf("n_args: %d", my_data->n_args);
            for(k=0; k<my_data->n_args; k++){
                th_data_array[i].args[k].opt = my_data->args[k].opt;
                th_data_array[i].args[k].value = my_data->args[k].value;

            }
                th_data_array[i].n_args = my_data->n_args;

            //printf("sou ficheiro\n");
            do_files(path, entry, th_data_array[i] ,&file_stat);

            if (S_ISDIR(file_stat.st_mode)) {
                //printf("diretorio:base_path: %s,  value:%s  n_thread: %d\n", my_data->base_path, th_data_array[k].args[0].value, th_data_array[k].n_thread);

                //do_files(path, entry, th_data_array[i] ,&file_stat);

                th_data_array[i].base_path = malloc(sizeof(char) * 300);

                strcat(path, "/");
                strcpy(th_data_array[i].base_path, path);
                //printf("diretorio:base_path: %s,  value:%s  n_thread: %d\n", th_data_array[i].base_path, th_data_array[i].args[0].value, th_data_array[i].n_thread);


                pthread_create(&th_data_array[i].thread_id, NULL, &do_directories, &th_data_array[i]);
                //printf("enviei thread %lu|!!\n", (unsigned long)th_data_array[i].thread_id);
                i++;
            }


      }

      strcpy(path, "");
    }
    closedir(dir);
  }

  for(j=0; j<i;j++){
    pthread_join(th_data_array[j].thread_id, NULL);
   //printf("\nthread{%d}_id - %lu \n", j, (unsigned long)th_data_array[j].thread_id);
  }


}


int main (int argc, char * argv[]) {

	int i=0;

    char *base_path = malloc(sizeof(char) * 300);

    T_DATA thread_main = { .base_path="", .args={NULL, ""}, .n_args=0 };


	T_DATA t_data = { .base_path="", .args={NULL, ""}, .n_args=0 };


    /*
	thread_main.base_path= argv[1];
	thread_main.args[t_data.n_args].opt = function_map[find_func_id(argv[2])].opt;
	thread_main.args[t_data.n_args].value = argv[3];
    thread_main.n_args++;
    */
	thread_main.base_path= argv[1];

    	for(i=2; i<argc; i+=2){

	thread_main.args[thread_main.n_args].opt = function_map[find_func_id(argv[i])].opt;
	thread_main.args[thread_main.n_args].value = argv[i+1];
	thread_main.n_thread=0;
    thread_main.n_args++;
	}

    printf("pre-thread\n");
    pthread_create (&thread_main.thread_id, NULL, &do_directories, &thread_main);
    pthread_join (thread_main.thread_id, NULL);

    /*
	t_data.base_path = argv[1];
	t_data.args[t_data.n_args].opt = function_map[find_func_id(argv[2])].opt;
	t_data.args[t_data.n_args].value = argv[3];
    t_data.n_args++;
*/
    /*
    t_data.args[t_data.n_args].opt = function_map[find_func_id(argv[4])].opt;
	t_data.args[t_data.n_args].value = argv[5];
	t_data.n_args++;
    */


	//do_directories(t_data.base_path, t_data);
	return 0;
}
