#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus;

int isLittle = 1;

virus* readVirus(FILE* sigFile){

    virus* inputVirus = (virus*)(malloc(sizeof(virus)));

    if (inputVirus == NULL) {
        perror("error in allocating memory\n");
        exit(1);
    }

    // reading sig size and virus name
    if(fread(inputVirus,1,18,sigFile)==18){

    inputVirus->sig = (unsigned char*)malloc(inputVirus->SigSize);
    if (inputVirus->sig == NULL) {
        perror("error in allocating memory for signature");
        free(inputVirus);
        exit(1);
    }
    else {
        if (isLittle==0) {
        inputVirus->SigSize = ((inputVirus->SigSize & 0xFF00) >> 8) | ((inputVirus->SigSize & 0x00FF) << 8);
        }
        if(fread(inputVirus->sig,1,inputVirus->SigSize,sigFile)!= inputVirus->SigSize){
        perror("error in reading to sig");
        free(inputVirus);
        exit(1); 
        }
    }

    }
    else
    {
        free(inputVirus);
        exit(1);
    }
    
    return inputVirus;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus Name: %s\n", virus->virusName);
    fprintf(output, "Signature Length: %d\n", virus->SigSize);
    fprintf(output, "Signature in hexadecimal: ");
    for (int i = 0; i < virus->SigSize; i++) {
        fprintf(output, "%02X ", virus->sig[i]);
    }
    fprintf(output, "\n\n");
}


int main(int argc, char* argv[]) {

    FILE* sigFile = fopen("signatures-B","rb");
    if (sigFile == NULL) {
        perror("Error opening file");
        return 1;
    }
    char magicNumber[4];
    fread(magicNumber, sizeof(char), 4, sigFile);
    if (strncmp(magicNumber, "VIRB", 4) == 0) {
        isLittle = 0;
    }
    else if(strncmp(magicNumber, "VIRL", 4) != 0) {
        fprintf(stderr, "Wrong magic number. Exiting.\n");
        fclose(sigFile);
        return 1;
    }
    while (!feof(sigFile)) {
        virus* v = readVirus(sigFile);
        printVirus(v, stdout);
        free(v->sig);
        free(v);
    }

    fclose(sigFile);


return 0;
}
