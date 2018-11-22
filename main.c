/*

PROVA FINALE DEL CORSO "ALGORITMI E PRINCIPI DELL'INFORMATICA"

REALIZZATO DA: JACOPO PIO GARGANO - 847989

*/
 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXDATA 100                             //the max number of characters you can write into the "data" attribute
#define INSTRUCTIONLENGTH 10                    //the max length of an instruction
#define NODESARRAYLENGTH 8392                   //hash map size
#define PATHSARRAYLENGTH 5                      //the initial length of the array of paths found by the "find" function
#define NAMELENGTH 256                          //the max number of characters of the name of a resource
#define MAXSONS 1024                            //the max amount of sons of a directory
#define SEPARATOR "/"                           //the separator in the path
#define PATHLENGTH 2048                         //the path's max length
#define MAXHEIGHT 255                           //the path's max heigth


//STRUCTS

enum bool {true, false};

struct node{                                                  //STRUCT NODE, a struct both for files and directories
    char name[NAMELENGTH];                                          //name
    char nodePath[PATHLENGTH];                                      //path
    enum bool isFile;                                               //flag that indicates whether or not the node is a file
    struct node * father;                                           //node's father
    struct node * son;                                              //node's first son (connects the node to its other sons)
    struct node * leftSibling;                                      //node's left sibling
    struct node * rightSibling;                                     //node's right sibling
    int numberOfSons;                                               //number of sons
    int height;                                                     //height
    char data[MAXDATA];                                             //the data you can write in
    struct node * nextInNodesArray;                                 //the next element in the big hash table
};

struct posAndLength{                                          //STRUCT used for the "find" function
    int pos;
    int length;
};




//GLOBAL VARIABLES

char instruction [INSTRUCTIONLENGTH];                           //current instruction
char path [PATHLENGTH];                                         //path of current instruction
char fathersPath [PATHLENGTH];                                  //current path trimmed by 1
char elementName [NAMELENGTH];                                  //the name of the last element in the current path
unsigned int hashValue;                                         //hash value associated to the current path
unsigned int fatherHashValue;                                   //hash value associated to the fathersPath
int pathLength;                                                 //the length of the current path
char c = '/';                                                   //separator
struct node * root;                                             //a global directory that holds all the others
struct node * nodesArray[NODESARRAYLENGTH];                     //hash map
char ** pathsArray;                                             //pointer to the array with the paths found by the "find" function




//AUXILIARY FUNCTIONS

int hash2 (const char* word)                                    //hash function
{
    unsigned int hash2 = 0;
    for (int i = 0 ; word[i] != '\0' ; i++)
    {
        hash2 = 31*hash2 + word[i];
    }
    return hash2 % NODESARRAYLENGTH;
}

//RETURNS A PATH TRIMMED BY 1 ELEMENT, ALSO GETS ITS NAME
void getFatherPath(char * par_path, char * par_futureFatherPath, char * par_elementName){
    strcpy(par_futureFatherPath, par_path);
    unsigned long len = strlen(par_path);
    unsigned long j;
    unsigned long i = 0;
    
    
    for(j=len-1; par_futureFatherPath[j]!='/'; j--){                  //until you find another separator
        par_futureFatherPath[j] = '\0';                               //move the end of string character back by 1 position
    }
    par_futureFatherPath[j] = '\0';
    unsigned long k=j+1;
    for(i=0; i<len-1-j; i++){                                         //get the name of the last element in the path
        elementName[i] = par_path[k];
        k++;
    }
    elementName[i] = '\0';
}

//GETS THE POINTER TO A SPECIFIED NODE (its path must be passed as a parameter)
struct node * getPointerToNode(char * par_path, unsigned int par_hashValue){
    struct node * temp = NULL;
    enum bool found = false;
    
    temp = nodesArray[par_hashValue];                               //gets the element in the hash map
    if(temp==NULL){                                                 //if there is nothing, return false
        return NULL;
    }
    if(!strcmp(temp->nodePath, par_path))                           //if its path is the same as the one passed through the parameter, return true
        found = true;
    while(found == false){                                          //else, look for it in the list
        temp = temp->nextInNodesArray;
        if(temp == NULL){
            return NULL;                                            //returns NULL if the object is not in the hash map
        }else{
            if(!strcmp(temp->nodePath, par_path))
                found = true;
        }
    }
    return temp;                                                    //returns the pointer to the node whose name is the last token in path
}

//GETS THE PATH'S LENGTH
int getPathLength(char * par_path){
    char temp_par_path[PATHLENGTH];                                 //workaround in order not to mess up with strings
    strcpy(temp_par_path, par_path);
    
    char * temp;
    int i = 0;
    char c = '/';
    temp = strchr(temp_par_path, c);
    if(temp == NULL) return 0;
    if(temp_par_path[0]!=c) return 0;
    temp = strtok(temp_par_path, "/");
    while(temp!=NULL){
        temp = strtok(NULL, "/");
        i++;
    }
    return i;
}

//ADD POINTERS TO STRINGS TO ARRAY OF PATHS FOUND BY THE "FIND" FUNCTION
struct posAndLength addToArrayOfPaths(struct node * par_node, char *** par_array, char * par_stringToCheck, int pos, int arrayLength){
    struct posAndLength temp;
    temp.pos = pos;
    temp.length = arrayLength;
    if(par_node!=NULL){
        if(par_node->isFile == true){                                             //A. if the node exists and it is a file
            if(par_node->rightSibling==NULL){                                                       //B. if it has no rightSibling
                if(!strcmp(par_node->name, par_stringToCheck)){                                     //C. if the node's name is the same as the string to check
                    if(temp.pos == temp.length){                                                         //if there is no more free space in the array
                        *par_array = realloc(*par_array, (arrayLength+PATHSARRAYLENGTH)* PATHLENGTH * sizeof(char *));                              //add PATHSARRAYLEGTH cells to the array
                        arrayLength = arrayLength + PATHSARRAYLENGTH;                                                              //increase the array's length by PATHSARRAYLEGTH
                    }
                    (*par_array)[temp.pos] = par_node->nodePath;                                                //put the pointer to the data in the array in position pos
                    pos++;                                                                          //increase pos
                    temp.pos = pos;
                    temp.length = arrayLength;
                    return temp;;                                                                   //returning the first free position and the array length
                }else{                                                                              //C. else, if the name is not the one searched for, return temp
                    return temp;
                }
            }else{                                                                                  //B. else if it has a rightSibling, execute the function on it
                temp = addToArrayOfPaths(par_node->rightSibling, par_array, par_stringToCheck, temp.pos, temp.length);
                if(!strcmp(par_node->name, par_stringToCheck)){                                     //do the same as above
                    if(temp.pos == temp.length){                                                         //if there is no more free space in the array
                        *par_array = realloc(*par_array, (arrayLength+PATHSARRAYLENGTH)* PATHLENGTH * sizeof(char *));                              //add PATHSARRAYLEGTH cells to the array                        arrayLength = arrayLength + PATHSARRAYLENGTH;                                                              //increase the array's length by PATHSARRAYLEGTH
                    }
                    (*par_array)[temp.pos] = par_node->nodePath;
                    temp.pos++;
                    temp.length = arrayLength;
                    return temp;
                }else{
                    return temp;
                }
            }
            
        }else{                                                                     //A. else, if it is a directory, execute function on rightSibling and sons
            if(par_node->rightSibling==NULL){                                      //B. if it does not have a rightSibling, check its son
                if(par_node->son==NULL){                                           //C. if it does not have a son, check directory
                    if(!strcmp(par_node->name, par_stringToCheck)){                                     //do the same as above
                        if(temp.pos == temp.length){                                                         //if there is no more free space in the array
                            *par_array = realloc(*par_array, (arrayLength+PATHSARRAYLENGTH)* PATHLENGTH * sizeof(char *));                              //add PATHSARRAYLEGTH cells to the array
                            arrayLength = arrayLength + PATHSARRAYLENGTH;                                                              //increase the array's length by PATHSARRAYLEGTH
                        }
                        (*par_array)[temp.pos] = par_node->nodePath;
                        temp.pos++;
                        temp.length = arrayLength;
                        return temp;
                    }else{
                        return temp;
                    }
                }else{                                                             //C. else, if it has a son, execute the function on its son
                    temp = addToArrayOfPaths(par_node->son, par_array, par_stringToCheck, temp.pos, temp.length);
                    if(!strcmp(par_node->name, par_stringToCheck)){                                     //do the same as above
                        if(temp.pos == temp.length){                                                         //if there is no more free space in the array
                            *par_array = realloc(*par_array, (arrayLength+PATHSARRAYLENGTH)* PATHLENGTH * sizeof(char *));                              //add PATHSARRAYLEGTH cells to the array
                            arrayLength = arrayLength + PATHSARRAYLENGTH;                                                              //increase the array's length by PATHSARRAYLEGTH
                        }
                        (*par_array)[temp.pos] = par_node->nodePath;
                        temp.pos++;
                        temp.length = arrayLength;
                        return temp;
                    }else{
                        return temp;
                    }
                }
            }else{                                                                 //B. else, if it has a rightSibling, execute on it, then eventually on its son
                temp = addToArrayOfPaths(par_node->rightSibling, par_array, par_stringToCheck, temp.pos, temp.length);
                if(par_node->son!=NULL){                                           //if it has a son, execute the function on its son
                    temp = addToArrayOfPaths(par_node->son, par_array, par_stringToCheck, temp.pos, temp.length);
                }
                if(!strcmp(par_node->name, par_stringToCheck)){                                     //do the same as above
                    if(temp.pos == temp.length){                                                         //if there is no more free space in the array
                        *par_array = realloc(*par_array, (arrayLength+PATHSARRAYLENGTH)* PATHLENGTH * sizeof(char *));                              //add PATHSARRAYLEGTH cells to the array
                        arrayLength = arrayLength + PATHSARRAYLENGTH;                                                              //increase the array's length by PATHSARRAYLEGTH
                    }
                    (*par_array)[temp.pos] = par_node->nodePath;
                    temp.pos++;
                    temp.length = arrayLength;
                    return temp;
                }else{
                    return temp;
                }
            }
        }
    }
    temp.pos = -1;
    temp.length = -1;
    return temp;                                                                   //function returns -1 if the node is NULL, should never happen if input is correct
}

//QUICKSORT ALGORITHM
int partition(char ** par_array, int first, int last){
    char element[PATHLENGTH];
    strcpy(element,par_array[last]);
    int i = first -1;
    int j= first;
    for(j = first; j<=last-1; j++){
        if(strcmp(element, par_array[j])>0){
            i++;
            char * temp;
            temp = par_array[i];
            par_array[i] = par_array[j];
            par_array[j] = temp;
        }
    }
    char * temp;
    temp = par_array[i+1];
    par_array[i+1] = par_array[j];
    par_array[j] = temp;
    return i+1;
}
enum bool quicksort(char ** par_array, int first, int last){
    if(first>last) return false;
    if(first==last) return true;
    
    int wall;
    wall = partition(par_array, first, last);
    quicksort(par_array, first, wall-1);
    quicksort(par_array, wall+1, last);
    
    return true;
}




//MAIN FUNCTIONS

//CREATE FILE
enum bool create(char * par_path, char * par_fathersPath, char * par_elementName, unsigned int par_hashValue, unsigned int par_fathersHashValue){
    struct node * father;
    struct node * element;
    
    if(!strcmp(par_fathersPath, par_path)) return false;
    if(!strcmp(par_fathersPath, "")){                                           //if the element has no father
        father = root;                                                          //its father is the root
        element = getPointerToNode(par_path, par_hashValue);                    //the element must not have been already created
        if(element != NULL) return false;                                       //if it already exists, return false
        
    }else{                                                                      //if the element is supposed to have a father
        father = getPointerToNode(par_fathersPath, par_fathersHashValue);       //get the father
        if(father == NULL){                                                     //if the father does not exist
            return false;                                                       //return false
        }
        element = getPointerToNode(par_path, par_hashValue);                    //the element must not have been already created
        if(element != NULL) return false;                                       //if it already exists, return false
    }
    
    struct node * file;                                                         //creating file
    file=malloc(sizeof(struct node));                                           //allocate memory
    if(file!=NULL){
        strcpy(file->name, par_elementName);                                    //set name
        strcpy(file->nodePath, par_path);                                       //set nodePath
        file->isFile = true;                                                    //set flag isFile
        file->father = father;                                                  //set father
        file->son = NULL;                                                       //setting son
        if(file->father->height==MAXHEIGHT){                                    //check the height
            free(file);
            return false;
        }else{
            file->height = file->father->height + 1;                            //set the height
        }
        if(file->father->numberOfSons==MAXSONS){                                //check that its father doesn't already have MAXSONS sons
            free(file);
            return false;
        }else{
            file->father->numberOfSons++;                                       //update father's number of sons
        }
        file->numberOfSons = 0;                                                 //set file number of sons
        if(file->father->son!=NULL){
            file->father->son->leftSibling = file;                              //set leftSibling for the element that shifts to the right
        }
        file->rightSibling = file->father->son;                                 //set file rightSibling as the father's previous son (even NULL, if it didn't have any)
        file->leftSibling = NULL;                                               //set file leftSibling as NULL
        file->father->son = file;                                               //set file's father son as the directory itself
        strcpy(file->data, "");
        if(nodesArray[par_hashValue] == NULL){                                  //check where to put the new element
            nodesArray[par_hashValue] = file;
            file->nextInNodesArray = NULL;
        }else{
            struct node * temp;
            temp = nodesArray[par_hashValue];
            while(temp->nextInNodesArray!=NULL){
                temp = temp->nextInNodesArray;
            }
            temp->nextInNodesArray = file;
            file->nextInNodesArray = NULL;
        }
        return true;
    }else{
        free(file);
    }
    return false;
}

//CREATE DIRECTORY
enum bool create_dir(char * par_path, char * par_fathersPath, char * par_elementName, unsigned int par_hashValue, unsigned int par_fathersHashValue){
    struct node * father;
    struct node * element;
    
    if(!strcmp(par_fathersPath, par_path)) return false;
    if(!strcmp(par_fathersPath, "")){                                           //if the element has no father
        father = root;                                                          //its father is the root
        element = getPointerToNode(par_path, par_hashValue);                    //the element must not have been already created
        if(element != NULL) return false;                                       //if it already exists, return false
        
    }else{                                                                      //if the element is supposed to have a father
        father = getPointerToNode(par_fathersPath, par_fathersHashValue);       //get the father
        if(father == NULL){                                                     //if the father does not exist
            return false;                                                       //return false
        }
        element = getPointerToNode(par_path, par_hashValue);                    //the element must not have been already created
        if(element != NULL) return false;                                       //if it already exists, return false
    }
    
    struct node * file;                                                         //creating file
    file=malloc(sizeof(struct node));                                           //allocate memory
    if(file!=NULL){
        strcpy(file->name, par_elementName);                                    //set name
        strcpy(file->nodePath, par_path);                                       //set nodePath
        file->isFile = false;                                                   //set flag isFile
        file->father = father;                                                  //set father
        file->son = NULL;                                                       //setting son
        if(file->father->height==MAXHEIGHT){                                    //check the height
            free(file);
            return false;
        }else{
            file->height = file->father->height + 1;                            //set the height
        }
        if(file->father->numberOfSons==MAXSONS){                                //checks that its father doesn't already have MAXSONS sons
            free(file);
            return false;
        }else{
            file->father->numberOfSons++;                                       //update father's number of sons
        }
        file->numberOfSons = 0;                                                 //set file number of sons
        if(file->father->son!=NULL){
            file->father->son->leftSibling = file;                              //set leftSibling for the element that shifts to the right
        }
        file->rightSibling = file->father->son;                                 //set file rightSibling as the father's previous son (even NULL, if it didn't have any)
        file->leftSibling = NULL;                                               //set file leftSibling as NULL
        file->father->son = file;                                               //set file's father son as the directory itself
        strcpy(file->data, "");
        if(nodesArray[par_hashValue] == NULL){                                  //check where to put the new element
            nodesArray[par_hashValue] = file;
            file->nextInNodesArray = NULL;
        }else{
            struct node * temp;
            temp = nodesArray[par_hashValue];
            while(temp->nextInNodesArray!=NULL){
                temp = temp->nextInNodesArray;
            }
            temp->nextInNodesArray = file;
            file->nextInNodesArray = NULL;
        }
        return true;
    }else{
        free(file);
    }
    return false;
}


//READ
char * readFunction (char * par_path){
    struct node * temp;
    
    temp = getPointerToNode(par_path, hash2(par_path));                         //get the element
    if(temp == NULL) return NULL;                                               //if it doesn't exist then you can't read it
    if(temp->isFile==false) return NULL;                                        //if it exists but it is NOT a file, you can't read it
    return temp->data;                                                          //if the file exists, then return its data to be printed
}

//WRITE
enum bool writeFunction (char * par_path, char * par_stringToBeWritten){
    struct node * temp;
    temp = getPointerToNode(par_path, hash2(par_path));                         //get the element
    if(temp == NULL) return false;                                              //if it doesn't exist then you can't write on it
    if(temp->isFile==false) return false;                                       //if it exists but it is NOT a file, you can't write on it
    strcpy(temp->data, par_stringToBeWritten);                                  //write on the file what it's needed to be written
    return true;
}


//DELETE
enum bool delete(char * par_path){
    struct node * temp;
    
    temp = getPointerToNode(par_path, hash2(par_path));
    if(temp==NULL) return false;                                             //if it doesn't exist then you can't delete it
    if(temp->son!=NULL) return false;                                        //if it has got sons then you can't delete it
    //UPDATE siblings and sons relationships
    if(temp->father->son == temp){                                           //if its father's son is temp itself
        temp->father->son = temp->rightSibling;                                 //father's son is temp's rightSibling
        if(temp->rightSibling!=NULL){                                           //if temp's rightSibling is NOT NULL, then
            temp->rightSibling->leftSibling = NULL;                                 //its rightSibling's leftSibling is set to NULL
        }
    }else{                                                                   //if its father's son is NOT temp itself
        temp->leftSibling->rightSibling = temp->rightSibling;                   //temp's leftSibling's rightSibling is set to temp's rightSibling
        if(temp->rightSibling!=NULL){                                           //if temp is not the last sibling
            temp->rightSibling->leftSibling = temp->leftSibling;                //temp's rightSibling's leftSibling is set to temp's leftSibling
        }
    }
    int hash = hash2(par_path);
    if(!strcmp(nodesArray[hash]->nodePath,par_path)){               //if what is found in nodesArray is the correct element (first in list)
        if(nodesArray[hash]->nextInNodesArray == NULL){               //if there is nothing connected to it
            nodesArray[hash] = NULL;
        }else{                                                                  //if there is something connected to it
            nodesArray[hash] = temp->nextInNodesArray;                    //move the list up
        }
    }else{                                                                    //else if temp is not the first in the list
        struct node * pred;
        pred = nodesArray[hash];                                      //set pred as the first in list
        while(pred->nextInNodesArray!=temp){              //go through the list until pred is actually temp's predecessor
            pred = pred->nextInNodesArray;
        }
        pred->nextInNodesArray = temp->nextInNodesArray;                    //update nextInNodesArray field for temp's predecessor using temp's successor (can be NULL)
    }
    free(temp);
    return true;
}

//DELETE_R
enum bool delete_r(char * par_path){
    struct node * temp;
    
    temp = getPointerToNode(par_path, hash2(par_path));                     //get the element
    if(temp==NULL) return false;                                            //if it doesn't exist then you can't delete it
    if(temp->son==NULL) return delete(par_path);                            //if it has got no sons then you can just delete it
    char pathSon[PATHLENGTH];
    enum bool flag = false;
    while(temp->son!=NULL){                                                 //while it has sons, execute "delete_r" on its son
        strcpy(pathSon, par_path);
        strcat(pathSon, "/");
        strcat(pathSon, temp->son->name);
        flag = delete_r(pathSon);
        if(flag == false) return flag;
    }
    flag = delete(par_path);                                                //when it has no more sons, delete the element
    return flag;
}

//FIND
void find(char * par_stringToBeFound){
    struct posAndLength researchVar;
    int i;
    researchVar = addToArrayOfPaths(root, &pathsArray, par_stringToBeFound, 0, PATHSARRAYLENGTH);      //put all the pointers to the paths with stringToBeFound in array
    if(researchVar.pos == -1){                                                                         //there has been an error
        printf("no\n");
        return;
    }
    if(researchVar.pos == 0){                                                                          //nothing was found
        printf("no\n");
        return;
    }
    enum bool flag = false;
    flag = quicksort(pathsArray, 0, researchVar.pos-1);                                                //order using quicksort the array of paths
    
    char print[PATHLENGTH];
    for(i=0; i<researchVar.pos; i++){                                                                  //print what was found
        strcpy(print,"");
        strcat(print, "ok ");
        strcat(print, pathsArray[i]);
        strcat(print, "\n");
        fputs(print, stdout);
    }
    return;
}

//MAIN
int main() {
    int i;
    
    //INITIALING VARIABLES
    for(i=0; i<NODESARRAYLENGTH; i++){                      //initializing nodesArray
        nodesArray[i]=NULL;
    }
    root = malloc(sizeof(struct node));                     //allocating memory for root
    if(root!=NULL){                                         //initializing root
        root->father = NULL;
        root->isFile = false;
        strcpy(root->name, "root");
        root->nextInNodesArray = NULL;
        strcpy(root->nodePath, "");
        root->numberOfSons = 0;
        root->height=0;
        root->rightSibling = NULL;
        root->leftSibling = NULL;
        root->son = NULL;
    }
    else{
        free(root);
        return -1;
    }
    
    scanf("%s", instruction);                               //read the instruction
    
    while (strcmp("exit", instruction)) {
        
        if (!strcmp(instruction, "create")){                        //CREATE FILE
            char symbol;
            symbol = getc(stdin);
            while(symbol== ' '){                                    //get all the spaces between the instruction and the path
                symbol = getc(stdin);
            }
            
            enum bool temp = false;
            if(symbol== '/'){
                strcpy(path, SEPARATOR);
                char tempPath[PATHLENGTH-1];
                gets(tempPath);
                strcat(path, tempPath);                             //put in "path" the current path
                getFatherPath(path, fathersPath, elementName);      //get the father's path
                hashValue = hash2(path);                            //hash the current path
                fatherHashValue = hash2(fathersPath);               //hash the father's path
                
                temp = false;
                temp = create(path, fathersPath, elementName, hashValue, fatherHashValue);
            }
            if(temp==true){
                printf("ok\n");
            }else{
                printf("no\n");
            }
        }
        else if (!strcmp("create_dir", instruction)){               //CREATE DIRECTORY
            char symbol;
            symbol = getc(stdin);
            while(symbol== ' '){                                    //get all the spaces between the instruction and the path
                symbol = getc(stdin);
            }
            enum bool temp = false;
            if(symbol== '/'){
                strcpy(path, SEPARATOR);
                char tempPath[PATHLENGTH-1];
                gets(tempPath);
                strcat(path, tempPath);                             //put in "path" the current path
                getFatherPath(path, fathersPath, elementName);      //get the father's path
                hashValue = hash2(path);                            //hash the current path
                fatherHashValue = hash2(fathersPath);               //hash the father's path
                
                temp = false;
                temp = create_dir(path, fathersPath, elementName, hashValue, fatherHashValue);
            }
            if(temp==true){
                printf("ok\n");
            }else{
                printf("no\n");
            }
            
        }
        else if (!strcmp("read", instruction)){
            scanf("%s",path);                                       //get the path
            char * content;
            content = readFunction(path);
            if(content == NULL){
                printf("no\n");
            }else{
                printf("contenuto %s\n", content);
            }
            
        }
        else if (!strcmp("write", instruction)){
            scanf("%s",path);                                       //get the path
            char content[MAXDATA];
            char symbol = '$';
            enum bool flag = false;
            scanf("%c",&symbol);                                    //get the space
            scanf("%c",&symbol);                                    //get the inverted commas
            i=1;
            fgets(content, MAXDATA, stdin);
            *(content + strlen(content)-2) = '\0';
            flag = writeFunction(path, content);
            if(flag == true){
                printf("ok %lu\n", strlen(content));
            }else
                printf("no\n");
        }
        else if (!strcmp("delete", instruction)){
            scanf("%s",path);                                       //get the path
            enum bool flag = false;
            flag = delete(path);
            if(flag == true){
                printf("ok\n");
            }else{
                printf("no\n");
            }
            
        }
        else if (!strcmp("delete_r", instruction)){
            scanf("%s",path);                                       //get the path
            enum bool flag = false;
            flag = delete_r(path);
            if(flag == true){
                printf("ok\n");
            }else{
                printf("no\n");
            }
        }
        else if (!strcmp("find", instruction)){
            pathsArray = calloc(PATHSARRAYLENGTH, PATHLENGTH * sizeof(char*));
            char stringToBeFound[NAMELENGTH];
            scanf("%s",stringToBeFound);
            if(strcmp(stringToBeFound,"")){
                find(stringToBeFound);
            }
            free(pathsArray);
        }
        else{
            //instruction not valid
            return -1;
        }
        scanf("%s", instruction);                           //read the instruction
    }
    free(root);
    return 0;
}
