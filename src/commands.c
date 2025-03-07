#include <../include/Label.h>
#include <../include/commands.h>

int isNegative(int num) {
    return (num & (1 << 23)) != 0;
}

int Mov(int *num1, int *num2){
    num2 = num1;
    return 0;
}

int Cmp(int *num1, int *num2){
    if(num1 == num2)
        return 1;
    return 0;
}

int Add(int *num1, int *num2){
    if()
    num2 += *num1;

    return 0;
}

int Sub(int *num1, int *num2){
    num2 -= *num1;

    return 0;
}

int Lea(Label *label, int *reg){

}

int Clr(int *reg){

}

int Not(int *num){
    
}

int Inc(int *num){
    
}

int Dec(int *num){
    
}

int Jmp(int *address){
    
}

int Bne(int *address){
    
}

int Jsr(int *address){
    
}

int Red(char chr){
    
}

int Prn(char chr){
    
}

int Rts(){
    
}

int Stop(){
    
}