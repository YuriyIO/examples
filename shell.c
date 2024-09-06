#include<stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include<string.h>
#include<fcntl.h>

int flag_amp;
int flag_larrow;
int flag_rarrow;
int flag_drarrow;
int flag_start;
int flag_fin;
int flag_status;
int flag_bracket;
int flag_read_br;
int flag_br_rarrow;
int flag_br_drarrow;

int* amp_pids = NULL;
int amp_len = 0;
int count;
int* pids = NULL;
int len = 0;
int fd_br[2];
int fd3[2];

char* read_str(int* check_EOF,int* str_length, FILE* file_in);
char** create_str( char** buf, int buf_length, int* word_count);
void print_str(char** str, int word_count);
int* add_pid(int* array, int*length, int pid);
void hander(int s);
void proc_pids();
void check_amp_pids();
char** set_cmd(char** str, int start, int fin);
void clean_str(char*** str, int word_count);
FILE* select_file(int argc, char** argv);


void rezero_flags()
{
    flag_fin = flag_start = 0;
}
void set_flag_amp(char** str, int* word_count)
{
    if (!strcmp(str[*word_count - 1], "&"))
    {
        flag_amp = 1;
        (*word_count)--;
        if((*word_count) == 0)
        {
            printf("some words expected\n");
            flag_amp = -1;
        }

    }
    else flag_amp = 0;
}

void set_flag_fin_start(char** str, int word_count)
{
    if(flag_fin == word_count)   
        return;
    if(flag_fin != 0)
        flag_start = flag_fin +1;
    int i = flag_start;
    for(; i < word_count; i++)
    {
        if(!strcmp(str[i], "|"))
            break;
    }
    flag_fin = i;
}

void set_flag_file(char** str)
{
    flag_rarrow = flag_drarrow = flag_larrow = 0;
    for(int i = flag_start; i < flag_fin; i++)
    {
        if(!strcmp(str[i], ">"))
        {
            flag_rarrow = i;
        }
        else if(!strcmp(str[i], ">>"))
        {
            flag_drarrow = i;
        }
        else if(!strcmp(str[i], "<"))
        {
            flag_larrow = i;
        }
    }
    
}

char** create_cmd(char** str)
{
    char** cmd = NULL;
    if(flag_larrow)
    {
            cmd = set_cmd(str, flag_start, flag_larrow);
    }
    else if(flag_rarrow)
    {
        cmd = set_cmd(str, flag_start, flag_rarrow);
    }
    else if(flag_drarrow)
    {
        cmd = set_cmd(str, flag_start, flag_drarrow);
    }
    else cmd = set_cmd(str, flag_start, flag_fin);
    return cmd;
}

int cmd_process(char** str, int word_count, char** full_str)
{
    int fd[2];
    int pid;
    char** cmd = NULL;
    if(flag_br_rarrow || flag_br_drarrow)
    {
        pipe(fd3);
    }
    while(flag_fin != word_count)
    {
        set_flag_fin_start(str, word_count);
        set_flag_file(str);
        cmd = create_cmd(str);
        pipe(fd);
        pid = fork();
        if(!pid)
        {
            signal(SIGINT, SIG_DFL);
            if(flag_larrow)
            {
                fd[0] = open(str[flag_larrow + 1], O_RDONLY);
                dup2(fd[0], 0);
            }
            if(flag_read_br)
            {
                dup2(fd_br[0], 0);
            }
            if(flag_br_rarrow)
            {
                if(!count)
                    fd3[1] = open(full_str[flag_br_rarrow + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
                else
                    fd3[1] = open(full_str[flag_br_rarrow + 1], O_APPEND | O_WRONLY);
                dup2(fd3[1],1);
            }
            else if(flag_br_drarrow)
            {
                fd3[1] = open(full_str[flag_br_drarrow + 1], O_APPEND | O_WRONLY);
                dup2(fd3[1],1);
            }
            else if(flag_bracket)
            {
                dup2(fd_br[1], 1);
            }
            else if(flag_rarrow)
            {
                fd[1] = open(str[flag_rarrow + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
                dup2(fd[1],1);
            }
            else if(flag_drarrow)
            {
                fd[1] = open(str[flag_drarrow + 1], O_APPEND | O_WRONLY);
                dup2(fd[1],1);
            }
            else if(flag_fin != word_count || flag_bracket)
            {
			    dup2(fd[1],1);
            }

            close(fd[0]);
            close(fd[1]);
            if(flag_bracket || flag_read_br)
            {
                close(fd_br[0]);
                close(fd_br[1]);
            }
            if(flag_br_rarrow || flag_br_drarrow)
            {
                close(fd3[0]);
                close(fd3[1]);
            }
            execvp(cmd[0], cmd);
            perror("2.2");
            exit(2);
        }
        free(cmd);
        cmd = NULL;
        if(flag_fin!= word_count || flag_bracket)
        {
            dup2(fd[0], 0);
        }
        close(fd[0]);
        close(fd[1]);
        if(flag_read_br)
        {
            close(fd_br[0]);
            close(fd_br[1]);
        }
        if(flag_br_rarrow || flag_br_drarrow)
        {
            count++;
            close(fd3[0]);
            close(fd3[1]);
        }

        if(flag_amp)
        {
            amp_pids = add_pid(amp_pids, &amp_len, pid);
            printf("child pid: [%d]\n", amp_pids[amp_len - 1]);
        }
        else
        {
            pids = add_pid(pids, &len, pid);
        }
    }
    while(len) pause();

    return 0;
}

void cmd_impl(char** str, int word_count, char** full_str)
{
    if(!strcmp(str[0], "cd"))
    {
        if(str[1] != NULL)
        {
            if(chdir(str[1]) != 0)
                perror(str[1]);
        }
        else chdir(getenv("HOME"));
    }
    else if(!strcmp(str[0], "exit"))
    {
        exit(0);
    }
    else
    {
        rezero_flags();
        set_flag_amp(str, &word_count);
        if(flag_amp != -1)
        {
            cmd_process(str, word_count, full_str);
        }
    }
}

int divide_str(char** str, int word_count, int* str_start, int* str_fin)
{
    if(*str_fin == word_count)
    {
        return 0;
    }
    if(*str_fin != 0)
    {
        *str_start = *str_fin + 1;
    }
    int i;
    for(i = *str_start; i < word_count; i++)
    {
        if(!strcmp(str[i], "||") || !strcmp(str[i], "&&") || !strcmp(str[i], ";"))
        {
            break;
        }
    }
    *str_fin = i;
    return 1;
}


void str_ex(char** str, int word_count, char** full_str)
{
    int str_start = 0;
    int str_fin = 0;
    char** tmp_str = NULL;
    divide_str(str, word_count, &str_start, &str_fin);
    while(1)
    {
        tmp_str = set_cmd(str, str_start, str_fin);
        cmd_impl(tmp_str, str_fin - str_start, full_str);
        if(str_fin == word_count)
        {
            free(tmp_str);
            tmp_str = NULL;
            break;
        }
        if((!strcmp(str[str_fin], "||") && (WIFEXITED(flag_status) && (WEXITSTATUS(flag_status) == 0))) || (!strcmp(str[str_fin], "&&") && !(WIFEXITED(flag_status) && (WEXITSTATUS(flag_status) == 0))))
        {
            free(tmp_str);
            tmp_str = NULL;
            break;
        }
        divide_str(str, word_count, &str_start, &str_fin);
        free(tmp_str);
        tmp_str = NULL;
    }
}

void find_br(char** str, int word_count, int* open_br, int* close_br)
{
    if(*close_br == word_count)
    {
        return;
    }

    if(*close_br != 0)
    {
        *open_br = *close_br + 1;
        while(!strcmp(str[*open_br], "(") || !strcmp(str[*open_br], ")") || !strcmp(str[*open_br], "|"))
            (*open_br)++;
    }
    else
    {
        while(!strcmp(str[*open_br], "("))
            (*open_br)++;
    }
    int i;
    for(i = *open_br; i < word_count; i++)
    {
        if(!strcmp(str[i],")") || !strcmp(str[i],"("))
            break;
    }
    *close_br= i;
    if(i == word_count)
    {
        flag_bracket = 0;
    }
    else flag_bracket = 1;
    if(*open_br != 0 && !strcmp(str[*open_br - 1], "|"))
    {
        flag_read_br = 1;
    }
    else flag_read_br = 0;
}

void proc_bracket(char** str, int* word_count)
{
    int open_br;
    int close_br;
    char** cmd = NULL;
    int i = 0;
    int save1 = dup(1);
    int save0 = dup(0);
    open_br = close_br = flag_bracket = flag_br_rarrow = flag_br_drarrow = count = 0;
    while(1)
    {
        find_br(str, *word_count, &open_br, &close_br);
        if(close_br != *word_count && i == 0)
        {
            pipe(fd_br);
            i++;
            if(open_br && !strcmp(str[*word_count - 2], ">"))
            {
                flag_br_rarrow = *word_count - 2;
                (*word_count)-=3;
            }
            if(open_br && !strcmp(str[*word_count - 2], ">>"))
            {
                flag_br_drarrow = *word_count - 2;
                (*word_count)-=3;
            }
        }
        if(!strcmp(str[close_br - 1], "||") || !strcmp(str[close_br - 1], "&&") || !strcmp(str[close_br - 1], ";"))
        {
            close_br--;
        }
        cmd = set_cmd(str, open_br, close_br);
        str_ex(cmd, close_br - open_br, str);
        if(close_br == *word_count)
        {
            free(cmd);
            cmd = NULL;
            dup2(save0,0);
	        dup2(save1,1);
	        close(save0);
	        close(save1); 
            break;
        }
        if(!strcmp(str[close_br + 1], "||") || !strcmp(str[close_br + 1], "&&") || !strcmp(str[close_br + 1], ";"))
            close_br++;
        if((!strcmp(str[close_br], "||") && (WIFEXITED(flag_status) && (WEXITSTATUS(flag_status) == 0))) || (!strcmp(str[close_br], "&&") && !(WIFEXITED(flag_status) && (WEXITSTATUS(flag_status) == 0))))
            find_br(str, *word_count, &open_br, &close_br);
        free(cmd);
        cmd = NULL;    
    }

}


int main(int argc,char* argv[])
{
    FILE* file_in = select_file(argc, argv);

    char** str = NULL;
    int word_count;

    int check_EOF;
    char* buf = NULL;
    int buf_length = 0;

    signal(SIGCHLD, proc_pids);
    signal(SIGINT, SIG_IGN);

    do 
    {
        printf(">");
        word_count = 0;
        buf = read_str(&check_EOF, &buf_length, file_in);
        str = create_str(&buf, buf_length, &word_count);
        if(str != NULL)
        {
            proc_bracket(str, &word_count);
            clean_str(&str, word_count);
        }
        free(buf);
        buf = NULL;
    } while (check_EOF != EOF);

    free(amp_pids);
    free(pids);
    fclose(file_in);

    return 0;
}

/////////////////////////////////////////////////
///////////////////////////////////////////
//////////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////////////////////////

/////////////////////////////////////////////////
///////////////////////////////////////////
//////////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////////////////////////

/////////////////////////////////////////////////
///////////////////////////////////////////
//////////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////////////////////////

/////////////////////////////////////////////////
///////////////////////////////////////////
//////////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////////////////////////

/////////////////////////////////////////////////
///////////////////////////////////////////
//////////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////////////////////////

/////////////////////////////////////////////////
///////////////////////////////////////////
//////////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////////////////////////

char* read_str(int* check_EOF,int* str_length, FILE* file_in)
{
    int c;
    int str_size = 0;
    int i = 0;
    char* str = NULL;
    int kol_quot = 0;

    while((c = getc(file_in)) != '\n' && c != EOF && c != '\r')
    {
        if(i >= str_size)
        {
            str_size = str_size*2 +1;
            str=(char*)realloc(str, str_size + 1);
        }
        str[i] = c;
        if(c == '"')
            kol_quot++;
        i++;
    }

    if(kol_quot % 2 != 0)
    {
        str[i] = 0;
        printf("Wrong string: %s\n",str);
        free(str);
        str = NULL;
        i = 0;
    }
    
    if(str != NULL)
    {
        str[i] = 0;
    }
    *str_length = i;
    *check_EOF = c;
    return str;
}


int separator(char c)
{
    return (c == ' ' || c == '&' || c == '|' || c == ';' || c == '>' || c == '<' || c == '(' || c == ')');
}

int double_symbl(char c)
{
    return (c == '&' || c == '|' || c == '>');
}

char* create_symb(int* j, int str_length, char* str, int* word_count)
{
    char* m = NULL;
    int size;
    if(((*j) < (str_length - 1)) && double_symbl(str[*j]) && (str[*j] == str[(*j) + 1]))
    {
        size=3;
        m=(char*)realloc(m, size);
        m[0] = str[*j];
        m[1] = str[(*j)+1];
        m[2] = 0;
        (*j)+=2;
    }
    else
    {
        size=2;
        m=(char*)realloc(m, size);
        m[0] = str[*j];
        m[1] = 0;
        (*j)++;
    }

    (*word_count)++;
    return m;
}

char* create_word(int* j, int str_length, char* str, int* word_count)
{
    char* m = NULL;
    int size = 0;
    int i = 0;

    while(!separator(str[*j]) && (*j) < str_length)
    {
        if(str[*j] != '"')
        {
            while(!separator(str[*j]) && str[*j] != '"' && (*j) < str_length)
            {
                if(i >= size)
                {
                    size = size*2 +1;
                    m = (char*)realloc(m, (size + 1) * sizeof(char));
                }
                m[i] = str[*j];
                (*j)++;
                i++;
            }
        }
        else
        {
            (*j)++;
            while(str[*j] != '"' && (*j) < str_length)
            {
                if(i >= size)
                {
                    size = size*2 +1;
                    m = (char*)realloc(m, (size + 1) * sizeof(char));
                }
                m[i] = str[*j];
                (*j)++;
                i++;
            }
            (*j)++;
        }
    }
    if(i == 0)
        return NULL; 
    m[i] = 0;
    (*word_count)++;
    return m;
}

void add_word_in_str(char*** str, int* str_length, char** current_word, int word_count)
{
    if(word_count>=*str_length)
        {
            *str_length = *str_length * 2 + 2;
            (*str) = (char**)realloc((*str), *str_length * sizeof(char*));
        }
    (*str) [word_count-1] = *current_word;
    *current_word = NULL;
}

char** create_str( char** buf, int buf_length, int* word_count)
{
    char** str = NULL;
    int str_length = 0;
    int j = 0;
    char* word = NULL;

    if(buf_length)
    {
        while(j < buf_length)
        {
            if((*buf) [j] == ' ')
            {
                j++;
            }
            else
            {
                if(separator((*buf) [j]))
                {
                    word = create_symb(&j, buf_length, *buf, word_count);
                }
                else
                {
                    word = create_word(&j, buf_length, *buf, word_count);
                }
                if(word != NULL)
                    add_word_in_str(&str, &str_length, &word, *word_count);
            }

        }
        if(*word_count)
            str[*word_count] = NULL;
    }
    return str;
}

void print_str(char** str, int word_count)
{
    int i;
    for(i=0; i < word_count; i++)
    {
        printf("%s\n", str[i]);
    }
}

int* add_pid(int* array, int* length,int pid)
{   int* m = array;
    m =(int*)realloc(m, (*length + 1) * sizeof(int));
    m[*length] = pid;
    (*length)++;
    return m;
}

int remove_pid(int** array, int* length, int pid)
{  
    int pid_ind = -5;
    int i;
    for (i = 0; i < *length; i++)
    {
        if ((*array)[i] == pid)
        {
            pid_ind = i;
            break;
        }
    }
    if (pid_ind >= 0)
    { 
        (*length)--;
        for (i = pid_ind; i < *length; i++)
        {
            (*array)[i] = (*array)[i+1];
        }
        return 1;
    }
    return 0;
}

void proc_pids()
{ 
    int pid;
    int search;
    int wstatus;
    do
    {
        pid = waitpid(-1, &wstatus, WNOHANG);  
        if (pid > 0)
        {
            search = remove_pid(&amp_pids, &amp_len, pid);
            if (search)
            {
                if (WIFEXITED(wstatus))
                    printf("[%d] done: status = %d\n", pid, WEXITSTATUS (wstatus));
                else if (WIFSIGNALED (wstatus))
                    printf("[%d] killed by signal %d\n", pid, WTERMSIG (wstatus)); 
                else if (WIFSTOPPED(wstatus))
                    printf("[%d] stopped by signal %d\n", pid, WSTOPSIG(wstatus));
                else if (WIFCONTINUED(wstatus))
                    printf("[%d] continued\n", pid);
            }
            else
            {
                search = remove_pid(&pids, &len, pid);
                if(search)
                {
                    flag_status = wstatus;
                }
            }
        }
        
    }
    while (pid > 0);
}


char** set_cmd(char** str, int start, int fin)
{
    char** tmp = (char**)malloc((fin - start + 1) * sizeof(char*));
    for(int i = start, j = 0; i < fin; i++, j++)
    {
        tmp[j] = str[i];
    }
    tmp[fin - start] = NULL;
    return tmp;
}

void clean_str(char*** str, int word_count)
{
    char** tmp = *str;
    for(int i = 0; i <= word_count; i++)
    {
        free(tmp[i]);
    }
    free(tmp);
    tmp = NULL;
}

FILE* select_file(int argc, char** argv)
{
    FILE* file_in;
    if(argc > 1)
    {
        if((file_in=fopen(argv[1],"r"))==NULL)
        {
            printf("File wasn't found, file_in == stdin\n");
            file_in = stdin;
        }
    }
    else file_in = stdin;
    return file_in;
}