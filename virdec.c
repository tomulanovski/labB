#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus;

typedef struct link link;

struct link {
link *nextVirus;
virus *vir;
};

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
        return NULL;
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

void list_print(link *virus_list, FILE* outputFile) {
    link *copyList = virus_list;
    while(copyList!=NULL) {
        printVirus(copyList->vir, outputFile);
        copyList = copyList ->nextVirus;
    }
}
link* list_append(link* virus_list, virus* data) {
    link* newVirus = (link*)malloc(sizeof(link));
     if (newVirus == NULL) {
        perror("error in allocating memory for new virus");
        exit(1);
    }
    newVirus ->vir = data; 
    newVirus->nextVirus = virus_list;
    return newVirus;
}

void list_free(link *virus_list) {
    while (virus_list !=NULL) {
        free(virus_list->vir->sig);
        free(virus_list->vir);
        link *tmp = virus_list->nextVirus;
        free(virus_list);
        virus_list = tmp;   
    }
}

link* loadSignatures(link *virus_list , char* inputFile) {
    char* fileName=NULL;
    char inBuf[BUFSIZ];
    printf("Enter signatures file name:\n");
    fgets(inBuf,sizeof(inBuf),stdin);
    sscanf(inBuf,"%ms",&fileName);
    FILE* sigFile = fopen(fileName,"rb");
    if (sigFile == NULL) {
        // freeing the ms allocation
        free(fileName);
        perror("no file");
        return NULL;
    }
    char magicNumber[4];
    fread(magicNumber, sizeof(char), 4, sigFile);
    if (strncmp(magicNumber, "VIRB", 4) == 0) {
        isLittle = 0;
    }
    else if(strncmp(magicNumber, "VIRL", 4) != 0) {
        fprintf(stderr, "Wrong magic number. Exiting.\n");
        fclose(sigFile);
        exit(1);
    }
     while (!feof(sigFile)) {
        virus* v = readVirus(sigFile);
        if(v!=NULL)
        virus_list = list_append(virus_list,v);
    }
    fclose(sigFile);
    // freeing the memory used by the %ms
    free(fileName);
    return virus_list;
}

link* printSignatures(link *virus_list , char* inputFile) {
    list_print(virus_list,stdout);
    return virus_list;
}
void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link *curVirus = virus_list;
    while (curVirus!=NULL) {
        virus* vir = curVirus ->vir;
        int virusSize = vir->SigSize;
        if (virusSize <= size) {
        for (int position = 0; position<size;position++) {
            
            if (memcmp(buffer+position, vir->sig, virusSize) == 0) {
                // Virus detected
                printf("Virus detected!\n");
                printf("Starting byte location: %d\n", position);
                printf("Virus name: %s\n", vir->virusName);
                printf("Size of the virus signature: %d\n", virusSize);
                break;
            }  
        }
        }
        curVirus = curVirus ->nextVirus;
    }
}

link* detectViruses(link *virus_list , char* inputFile){
    char buf[10000*sizeof(char)];
    FILE *suspectedFile = fopen(inputFile, "rb");
    if (suspectedFile == NULL) {
        perror("Error opening suspected file");
        return virus_list;
    }
    unsigned int size = fread(buf, 1, sizeof(buf), suspectedFile);
    detect_virus(buf, size, virus_list);
    fclose(suspectedFile);
    return virus_list;
}
void neutralize_virus(char *fileName, int signatureOffset){
    FILE *infectedFile = fopen(fileName, "r+b");

    if (infectedFile == NULL) {
        perror("Error opening infected file");
        return;
    }

    // Move the file pointer to the location of the virus in the file
    fseek(infectedFile, signatureOffset, SEEK_SET);

    // Neutralize the virus by writing a RET instruction (e.g., 0xC3) at the offset
    char RET[] = "0xC3";
    fwrite(RET, sizeof(RET), 1, infectedFile);
    fclose(infectedFile);

}



link* fixFile(link *virus_list , char* inputFile){
    char buf[10000 * sizeof(char)];
    FILE *suspectedFile = fopen(inputFile, "rb");

    if (suspectedFile == NULL) {
        perror("Error opening suspected file");
        return virus_list;
    }

    unsigned int size = fread(buf, 1, sizeof(buf), suspectedFile);
    fclose(suspectedFile);

    link *curVirus = virus_list;

    while (curVirus != NULL) {
        virus *vir = curVirus->vir;
        int virusSize = vir->SigSize;

        for (int position = 0; position < size - virusSize; position++) {
            if (memcmp(buf + position, vir->sig, virusSize) == 0) {
                // Virus detected
                printf("Virus %s detected!\n",vir->virusName);
                printf("neutralizing the virus\n");
                // Neutralize the virus in the infected file
                neutralize_virus(inputFile, position);
                printf("virus %s neutralized \n", vir->virusName);
                break;
            }
        }

        curVirus = curVirus->nextVirus;
    }

    return virus_list;
}
link* quit(link *virus_list , char* inputFile) {
    list_free(virus_list);
    printf("quitting\n");
    return NULL;
}
struct fun_desc {
    char *name;
    link* (*fun)(link* ,char*);
};


int main(int argc, char* argv[]) {

    if(argc!=2) {
        printf(" number of arguments is not 2");
        exit(1);
    }

    
       struct fun_desc menu[] = {
            {"Load signatures", loadSignatures},
            {"Print signatures", printSignatures},
            {"Detect viruses", detectViruses},
            {"Fix file", fixFile},
            {"quit", quit}, 
            {NULL,NULL}      
    };
        char input[10];
        int bound = sizeof(menu) / (sizeof(menu[0]))-1;
        int opChosen;
        link *virus_list = NULL;
        while(1) {

            printf("choose an option:\n");
            for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d- %s\n", i+1, menu[i].name);
        }
         if (fgets(input, sizeof(input), stdin) == NULL) {
            // Exit the loop on EOF
            printf("Exiting\n");
            exit(1);
        }
        sscanf(input,"%d",&opChosen);
        opChosen--;
        if(opChosen==4) {
            quit(virus_list,argv[1]);
            exit(0);

        }
        if (opChosen>=0 && opChosen<bound) {
            printf (" within bounds\n");
            virus_list = menu[opChosen].fun(virus_list,argv[1]);
        }
        else {
            printf ("Not within bounds\nExiting\n");
            exit(1);
        }
        }
return 0;
}


