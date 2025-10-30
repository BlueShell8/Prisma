#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS 100
#define MAX_NAME 50
#define MAX_LINE 256

typedef struct {
    char name[MAX_NAME];
    int value;
} Variable;

Variable vars[MAX_VARS];
int var_count = 0;

// Variable auslesen
int get_var(char* name) {
    for(int i=0;i<var_count;i++)
        if(strcmp(vars[i].name,name)==0) return vars[i].value;
    return 0;
}

// Variable setzen
void set_var(char* name, int value) {
    for(int i=0;i<var_count;i++) {
        if(strcmp(vars[i].name,name)==0) {
            vars[i].value = value;
            return;
        }
    }
    strcpy(vars[var_count].name,name);
    vars[var_count].value = value;
    var_count++;
}

// Bedingung auswerten
int eval_condition(char* cond) {
    char var[MAX_NAME]; int num;
    if(sscanf(cond,"%s größer als %d",var,&num)==2)
        return get_var(var) > num;
    if(sscanf(cond,"%s kleiner als %d",var,&num)==2)
        return get_var(var) < num;
    if(sscanf(cond,"%s gleich %d",var,&num)==2)
        return get_var(var) == num;
    return 0;
}

// Zeilen ausführen
void execute_lines(char lines[][MAX_LINE], int start, int end) {
    for(int i=start;i<end;i++) {
        char* line = lines[i];
        if(strncmp(line,"setze ",6)==0) {
            char name[MAX_NAME]; int value;
            sscanf(line+6,"%s auf %d",name,&value);
            set_var(name,value);
        } else if(strncmp(line,"zeige ",6)==0) {
            char out[MAX_LINE];
            if(sscanf(line+6,"\"%[^\"]\"",out)==1) {
                printf("%s\n",out);
            } else {
                char var[MAX_NAME];
                sscanf(line+6,"%s",var);
                printf("%d\n",get_var(var));
            }
        } else if(strncmp(line,"wenn ",5)==0) {
            int j=i+1, else_line=-1, end_line=-1;
            while(j<end) {
                if(strcmp(lines[j],"ende")==0) { end_line=j; break; }
                if(strcmp(lines[j],"sonst")==0) else_line=j;
                j++;
            }
            char cond[MAX_LINE];
            strncpy(cond,line+5,MAX_LINE);
            if(eval_condition(cond)) {
                execute_lines(lines,i+1, else_line!=-1? else_line : end_line);
            } else if(else_line!=-1) {
                execute_lines(lines,else_line+1,end_line);
            }
            i = end_line;
        } else if(strncmp(line,"wiederhole solange ",18)==0) {
            int j=i+1, end_line=-1;
            while(j<end) { if(strcmp(lines[j],"ende")==0) {end_line=j; break;} j++; }
            char cond[MAX_LINE];
            strncpy(cond,line+18,MAX_LINE);
            while(eval_condition(cond)) {
                execute_lines(lines,i+1,end_line);
            }
            i=end_line;
        }
    }
}

// Datei einlesen
void execute_file(const char* filename) {
    FILE* f = fopen(filename,"r");
    if(!f) { printf("Datei nicht gefunden\n"); return; }
    char lines[1000][MAX_LINE];
    int count=0;
    while(fgets(lines[count],MAX_LINE,f)) {
        lines[count][strcspn(lines[count], "\r\n")] = 0; // Zeilenumbruch entfernen
        count++;
    }
    fclose(f);
    execute_lines(lines,0,count);
}

int main(int argc, char* argv[]) {
    if(argc<2) { printf("Verwendung: %s datei.pris\n",argv[0]); return 0; }
    execute_file(argv[1]);
    return 0;
}
