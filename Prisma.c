#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS 100
#define MAX_FUNCS 50
#define MAX_NAME 50
#define MAX_LINE 256
#define MAX_LINES 1000

typedef struct {
    char name[MAX_NAME];
    int value;
} Variable;

typedef struct {
    char name[MAX_NAME];
    int param_count;
    char param_names[10][MAX_NAME];
    char lines[MAX_LINES][MAX_LINE];
    int line_count;
} Function;

Variable vars[MAX_VARS];
int var_count = 0;

Function funcs[MAX_FUNCS];
int func_count = 0;

// Variablen setzen
void set_var(char* name, int value) {
    for(int i=0;i<var_count;i++){
        if(strcmp(vars[i].name,name)==0){
            vars[i].value = value;
            return;
        }
    }
    strcpy(vars[var_count].name,name);
    vars[var_count].value = value;
    var_count++;
}

// Variablen auslesen
int get_var(char* name){
    for(int i=0;i<var_count;i++)
        if(strcmp(vars[i].name,name)==0) return vars[i].value;
    return 0;
}

// Ausdruck auswerten (einfach +, -, *, /)
int eval_expr(char* expr){
    int a,b;
    char var[MAX_NAME], op;
    if(sscanf(expr,"%s %c %d",var,&op,&b)==3){
        a = get_var(var);
        if(op=='+') return a+b;
        if(op=='-') return a-b;
        if(op=='*') return a*b;
        if(op=='/') return b!=0?a/b:0;
    } else if(sscanf(expr,"%d",&a)==1){
        return a;
    } else {
        return get_var(expr);
    }
    return 0;
}

// Bedingung auswerten
int eval_condition(char* cond){
    char var[MAX_NAME]; int num;
    if(sscanf(cond,"%s größer als %d",var,&num)==2) return get_var(var) > num;
    if(sscanf(cond,"%s kleiner als %d",var,&num)==2) return get_var(var) < num;
    if(sscanf(cond,"%s gleich %d",var,&num)==2) return get_var(var) == num;
    return 0;
}

// Funktionsdefinition speichern
void define_function(char lines[MAX_LINES][MAX_LINE], int* i, int total){
    char name[MAX_NAME];
    int param_count=0;
    char params[10][MAX_NAME];
    sscanf(lines[*i]+9,"%s",name); // "funktion " = 9
    char* p = strchr(lines[*i],'(');
    char* q = strchr(lines[*i],')');
    if(p && q && q>p){
        char param_str[MAX_LINE];
        strncpy(param_str,p+1,q-p-1);
        param_str[q-p-1] = 0;
        char* tok = strtok(param_str,",");
        while(tok){
            strcpy(params[param_count],tok);
            param_count++;
            tok = strtok(NULL,",");
        }
    }
    // Linien bis "ende" speichern
    int start = *i +1;
    int lc=0;
    int j=start;
    while(j<total && strcmp(lines[j],"ende")!=0){
        strcpy(funcs[func_count].lines[lc],lines[j]);
        lc++; j++;
    }
    strcpy(funcs[func_count].name,name);
    funcs[func_count].param_count = param_count;
    for(int k=0;k<param_count;k++)
        strcpy(funcs[func_count].param_names[k],params[k]);
    funcs[func_count].line_count = lc;
    func_count++;
    *i = j; // Sprung zu ende
}

// Funktionsaufruf
void call_function(char* name, char* args){
    for(int f=0;f<func_count;f++){
        if(strcmp(funcs[f].name,name)==0){
            // lokale Variablen speichern
            Variable old_vars[MAX_VARS]; int old_count = var_count;
            memcpy(old_vars,vars,sizeof(vars));
            // Argumente zuweisen
            char* tok = strtok(args,",");
            int idx=0;
            while(tok && idx<funcs[f].param_count){
                set_var(funcs[f].param_names[idx], eval_expr(tok));
                tok = strtok(NULL,",");
                idx++;
            }
            // Funktion ausführen
            for(int l=0;l<funcs[f].line_count;l++){
                char line[MAX_LINE]; strcpy(line,funcs[f].lines[l]);
                if(strncmp(line,"setze ",6)==0){
                    char n[MAX_NAME]; char val[MAX_LINE];
                    sscanf(line+6,"%s auf %[^\n]",n,val);
                    set_var(n,eval_expr(val));
                } else if(strncmp(line,"zeige ",6)==0){
                    char out[MAX_LINE];
                    if(sscanf(line+6,"\"%[^\"]\"",out)==1)
                        printf("%s\n",out);
                    else
                        printf("%d\n",eval_expr(line+6));
                }
            }
            // alte Variablen wiederherstellen
            memcpy(vars,old_vars,sizeof(vars));
            var_count = old_count;
        }
    }
}

// Ausführen von Lines
void execute_lines(char lines[MAX_LINES][MAX_LINE], int total){
    for(int i=0;i<total;i++){
        char* line = lines[i];
        if(strncmp(line,"setze ",6)==0){
            char name[MAX_NAME]; char val[MAX_LINE];
            sscanf(line+6,"%s auf %[^\n]",name,val);
            set_var(name,eval_expr(val));
        } else if(strncmp(line,"zeige ",6)==0){
            char out[MAX_LINE];
            if(sscanf(line+6,"\"%[^\"]\"",out)==1) printf("%s\n",out);
            else printf("%d\n",eval_expr(line+6));
        } else if(strncmp(line,"wenn ",5)==0){
            int j=i+1, else_line=-1, end_line=-1;
            while(j<total){
                if(strcmp(lines[j],"ende")==0){ end_line=j; break; }
                if(strcmp(lines[j],"sonst")==0) else_line=j;
                j++;
            }
            char cond[MAX_LINE]; strncpy(cond,line+5,MAX_LINE);
            if(eval_condition(cond))
                execute_lines(lines,i+1,else_line!=-1? else_line: end_line);
            else if(else_line!=-1)
                execute_lines(lines,else_line+1,end_line);
            i = end_line;
        } else if(strncmp(line,"wiederhole solange ",18)==0){
            int j=i+1, end_line=-1;
            while(j<total){ if(strcmp(lines[j],"ende")==0){ end_line=j; break;} j++; }
            char cond[MAX_LINE]; strncpy(cond,line+18,MAX_LINE);
            while(eval_condition(cond))
                execute_lines(lines,i+1,end_line);
            i=end_line;
        } else if(strncmp(line,"funktion ",9)==0){
            define_function(lines,&i,total);
        } else {
            // Funktionsaufruf prüfen
            char fname[MAX_NAME], args[MAX_LINE];
            if(sscanf(line,"%s(%[^)]s)",fname,args)==2)
                call_function(fname,args);
        }
    }
}

// Datei ausführen
void execute_file(const char* filename){
    FILE* f = fopen(filename,"r");
    if(!f){ printf("Datei nicht gefunden\n"); return; }
    char lines[MAX_LINES][MAX_LINE]; int count=0;
    while(fgets(lines[count],MAX_LINE,f)){
        lines[count][strcspn(lines[count],"\r\n")]=0;
        count++;
    }
    fclose(f);
    execute_lines(lines,count);
}

int main(int argc, char* argv[]){
    if(argc<2){ printf("Verwendung: %s datei.pris\n",argv[0]); return 0; }
    execute_file(argv[1]);
    return 0;
}
