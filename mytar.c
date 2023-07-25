#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

/*
| INT |    METADATA   |         DATA          |
*/
typedef struct{
    char    name[20];      /* file name as a null-terminated string */ 
    int     size;          /* the size of the file in bytes */
    int     offset;        /* the location (offset from the start of the archive file in bytes) of the file placed in the archive */
}entry;

void printError(int error){
    int test = error;
    switch (test){
        case 1:
            printf("Usage: mytar {-t | -c | -x | -u} -f <archive_file_name> [list_of_files] \n");
            exit(1);
            break;
        case 2:
            printf("ERROR: One or more files exceeded name character limit (19).\n");
            exit(1);
            break;
        case 3:
            printf("ERROR: Exceeded file limit (20). \n");
            exit(1);
            break;
        case 4:
            printf("ERROR: Option requires one or more files to be input. \n");
            exit(1);
            break;
        case 5:
            printf("ERROR: One or more files that you want to archive do not exist.\n");
            exit(1);
            break;
        case 6:
            printf("ERROR: One or more files that you want to archive are directories.\n");
            exit(1);
            break;
        case 7:
            printf("ERROR: One or more files with the same name already exist in the binary.\n");
            exit(1);
        case 8:
            printf("ERROR: Invalid bin file.\n");
            exit(1);
        case 9:
            printf("ERROR: One or more duplicate files in the input.\n");
            exit(1);
    }
}

void x(char* binName){
    int     nFiles;
    int     tempSize;
    int     tempOffSet;
    int     stop;
    
    char    tempName    [20];
    char    buf         [1];

    FILE*   tempPtr;
    FILE*   bin = fopen(binName, "r");

    fread(&nFiles, sizeof(int), 1, bin);
    
    entry   dir[nFiles];
    
    for(int i = 0; i < nFiles; i++){
        fseek(bin, (i*28) + 4, SEEK_SET);
        fread(tempName, sizeof(tempName), 1, bin);
        strcpy(dir[i].name, tempName);

        fread(&tempSize, sizeof(int), 1, bin);
        dir[i].size = tempSize;
        fread(&tempOffSet, sizeof(int), 1, bin);
        dir[i].offset = tempOffSet;

        tempPtr = fopen(tempName, "r");
        
        if(tempPtr != NULL){
            fclose(tempPtr);
            printf("File %s already exists in the current directory. \n", dir[i].name);
            continue;
        }

        fseek(bin, dir[i].offset, SEEK_SET);
        tempPtr = fopen(dir[i].name,"w");

        stop = dir[i].size + dir[i].offset;
        printf ("stop %d\n", stop);

        while((int)ftell(bin) < stop){
            fread(buf, sizeof(buf), 1, bin);
            fwrite(buf, sizeof(buf), 1, tempPtr);
        }
        printf("File extracted %d/%d: %s (%dB)\n", i+1, nFiles,dir[i].name, dir[i].size);
        strcpy(buf,"");

        fclose(tempPtr);
    } 

    fclose(bin);
}

//https://stackoverflow.com/questions/3757899/sorting-strings-using-qsort
int cmpStr(const void *a, const void *b){
    char const *aa = (char const *)a;
    char const *bb = (char const *)b;

    return strcmp(aa, bb);
}
void t(char* binName){
        int     files;
        entry   read[files];

        FILE*  bin = fopen(binName, "r");
        fread(&files, sizeof(int), 1, bin);
        
        for(int i = 0; i < files; i++){
            //set instead of curr for subsequent 
            fseek(bin, ((i*28) + 4), SEEK_SET);
            
            fread(read[i].name, sizeof(char), 20, bin);
            fread(&read[i].size, sizeof(int), 1, bin);
        }

        qsort(&read[0], files, sizeof(entry), cmpStr);
        printf("Number of files in the binary: %d\n", files);

        for(int i = 0; i < files; i++){
            printf("<File: \t %s \t of size:\t %dB>\n", read[i].name, read[i].size);
        }

        fclose(bin);
}

void main(int argc, char *argv[]){

    FILE *  bin;
    FILE *  tempPtr;
    
    struct stat sysStat;

    char *  binName;
    char *  tempName;
    
    char    buf[4096];
    int     tempSize;
    
    

    //option index, and 
    int option, cf, ff, uf, m, flagSum;
    //flags set to 0
    flagSum = uf = cf = ff = m = 0; 
    
    while ((option = getopt(argc, argv, "tcxuf:")) != -1){
        switch (option){
            case 'f':
            //file
                if(flagSum == 0){
                    printf("Please print the options in the correct order.\n");
                    printError(1);
                }
                binName = optarg;
                ff = 1;
                break;
            case 'c':
            //Append
                if(flagSum > 0){
                    printError(1);
                }
                cf = 1;
                break;
            case 'u':
            //Extract 
                if(flagSum > 0){
                    printError(1);
                }
                uf = 1;
                break;
            case 'x':
            //Create
                if(flagSum > 0){
                    printError(1);
                }
                m = 1;
                break;
            case 't':
            //List
                if(flagSum > 0){
                    printError(1);
                }
                m = 2;
                break;
        }
        flagSum = m + uf + cf;
    }
    
    //argc minus all of the options
    argc -= optind;
    //argv moved to the position of the first file
    argv += optind;
    
    //if f is not selected
    if (!ff){
        printError(1);
    }
    //if more than 20 files
    if (argc > 20){
        printError(3);
    } 

    for(int i = 0; i < argc; i++){
            //if name is larger than 19
            if(strlen(argv[i]) > 19){
                printError(2);
            }
            stat(argv[i], &sysStat);
            if(S_ISDIR(sysStat.st_mode)){
                printError(6);
            }

            tempPtr = fopen(argv[i], "r");
            if(tempPtr == NULL){
                printError(5);
            }else{
                fclose(tempPtr);
            }
    };

    
    
    //create
    //not a separate method
    //relies on argv
    if(cf){
        entry   directory[20];
        int     offset = 564;
        
        if (argc < 1){
            printError(4);
        }
        for(int i = 1; i < argc; i++){
            for(int j = i - 1; j > -1; j--){
                if(!strcmp(argv[i], argv[j])){
                    printError(9);
                }
            }
        }

        bin = fopen(binName, "wb");                    
        
        fwrite(&argc, sizeof(int), 1, bin);            
        fseek(bin, 4, SEEK_SET);                       
        
        for (int i = 0; i < argc; i++){
            tempName = argv[i];
            tempPtr = fopen(tempName, "r");     

            strcpy(directory[i].name, tempName);

            fseek(tempPtr, 0, SEEK_END);               
            tempSize = (int)ftell(tempPtr);            
            directory[i].size = tempSize;
            
            directory[i].offset = offset;              
            
            fseek(bin, offset, SEEK_SET);               
            fseek(tempPtr, 0, SEEK_SET);                
        
            while(!feof(tempPtr)){                      
                fread(buf, sizeof(buf), 1, tempPtr);     
                fwrite(buf, sizeof(buf), 1, bin);
                strcpy(buf,"");       
            }

            offset += tempSize;                         
            fclose (tempPtr);                           
        }

        fseek(bin, 4, SEEK_SET);                       
        //actual object is of size nFiles, but the 
        //amount of bytes carved out is 20 * size of the struct for future appends. 
        fwrite(&directory, sizeof(entry)*20, 1, bin);   
        fclose(bin);                                       
    }
    
    //not a separate method as well
    //relies on argv 
    if(uf){
        int     nFiles;
        int     lastEntry; 
        int     dirLSize;
        int     dirLOffset;
        char    nameTest[20];
        entry   directory[argc];
        
        //data variables
        int offset;
        
        //checks that at least one file is being added
        if (argc < 1){
            printError(4);
        }

        //checks that the user uses a valid file
        tempPtr = fopen(binName, "r");
            if(tempPtr == NULL){
                printError(8);
            }else{
                fclose(tempPtr);
            }

        bin = fopen(binName,"rb+");

        //reads first integer
        fread(&nFiles, sizeof(int), 1, bin);

        //bruteforce to find for repeat in the input
        for(int i = 1; i < argc; i++){
            for(int j = i - 1; j > -1; j--){
                if(!strcmp(argv[i], argv[j])){
                    printError(9);
                }
            }
        }
        
        //bruteforce check for repeat files
        for(int i = 0; i < argc; i++){
            for(int j = 0; j < nFiles; j++){
                fseek(bin, j*28 + 4, SEEK_SET);
                fread(nameTest, sizeof(nameTest), 1, bin);
                if(!strcmp(argv[i], nameTest)){
                    fclose(bin);
                    printError(7);
                }
            }
        }

        

        //checks if the file can handle more files.
        if((nFiles + argc) > 20){
            fclose(bin);
            printError(3);
        }

        lastEntry = (28 * nFiles);
        fseek(bin, 4, SEEK_SET);
        fseek(bin, lastEntry - 8, SEEK_CUR);
        
        fread(&dirLSize, sizeof(int), 1, bin);
        fread(&dirLOffset, sizeof(int), 1, bin);
        
        offset = dirLOffset + dirLSize;

        //append starts at the end of the last file in data
        fseek(bin, offset, SEEK_SET);
        for(int i = 0; i < argc; i++){
            tempName = argv[i];
            tempPtr = fopen(tempName, "r");     

            strcpy(directory[i].name, tempName);

            fseek(tempPtr, 0, SEEK_END);               
            tempSize = (int)ftell(tempPtr);            
            directory[i].size = tempSize;
            
            directory[i].offset = offset;              
            
            fseek(bin, offset, SEEK_SET);               
            fseek(tempPtr, 0, SEEK_SET);

            while(!feof(tempPtr)){                      
                fread(buf, sizeof(buf), 1, tempPtr);     
                fwrite(buf, sizeof(buf), 1, bin); 
                strcpy(buf,"");       
            }

            offset += tempSize;                    
            fclose (tempPtr);  
        }
        
        //update number of files
        nFiles += argc;
        fseek(bin, 0, SEEK_SET);
        fwrite(&nFiles, sizeof(int), 1, bin);
        
        //append the directories
        fseek(bin, lastEntry, SEEK_CUR);
        fwrite(&directory, sizeof(entry)*argc, 1, bin);

        fclose(bin); 
    }

    switch(m){
        case 1:
            x(binName);
            break;
        case 2:
            t(binName);
            break;
    }

}