
// including header files
#include "fat32.h"
#include "DirInfo.h"
#include "Fsinfo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// constant
#define MAX 50

//declaring prototypes
void printInfo(struct FAT32_INFO *);
int fetchData(int, char *, struct FAT32_INFO *, struct Fsinfo *, char *, char *);
int parseBootSector(int, int, struct FAT32_INFO *, struct Fsinfo *, char *, char *);
void traverseDirectoryTree(int, int, uint32_t, struct FAT32_INFO *, int);
int getClusterIndex(int, struct FAT32_INFO *);
uint64_t getFatEntry(uint32_t, int, struct FAT32_INFO *);
void traverseAndReadRecursive(int, int, struct DirInfo *, struct FAT32_INFO *, uint32_t, int, int);
int getDataClusters(int, int, uint32_t, struct FAT32_INFO *, char **, int);
void parseDirectoryPath(int, int, uint32_t, struct FAT32_INFO *, char *);


// Main method
int main(int argc, char *argv[]){
    int process = 0;
    printf("\n------------------>Reading Fat32 formatted drive<----------------\n\n");
    if (argc < 3){
        printf("\nWrong arguments entered.\n");
        printf("\nSequence expected : ./a3 <imagename> <function_name>\n\n");
        return (1);
    }
    else{
        struct FAT32_INFO *fatInfo = NULL;
        struct Fsinfo *fsInfo = NULL;
        if (argc == 3){
            process = fetchData(0, argv[1], fatInfo, fsInfo, argv[2], NULL);
        }
        else if (argc == 4){
            process = fetchData(0, argv[1], fatInfo, fsInfo, argv[2], argv[3]);
        }
    }
    if (process == -1){
        printf("FAT32 not successful!\n");
    }
    return 0;
}

// Calculating and Printing the info about the FAT32 FS
void printInfo(struct FAT32_INFO *fatInfo){
    int size = fatInfo->free_clusters * fatInfo->sector_per_cluster * fatInfo->bytes_per_sector / 1024.0;
    int usable = fatInfo->used_clusters + fatInfo->free_clusters - fatInfo->bad_clusters;
    int usableSpace = usable * fatInfo->sector_per_cluster * fatInfo->bytes_per_sector / 1024.0;

    printf("\nInfo about the Fat 32 file system\n\n");
    printf("    => OEM Name : %s\n", fatInfo->oem_name);
    printf("    => Volume Label : %s\n", fatInfo->volume_label);
    printf("    => Free space on the drive (in KB) : %d\n", size);
    printf("    => Amount of usable storage on the drive (in KB) : %d\n", usableSpace);
    printf("    => Cluster size (in sectors) : %d sectors\n", fatInfo->sector_per_cluster);
    printf("    => Cluster size (in KB) : %f\n\n", fatInfo->sector_per_cluster * fatInfo->bytes_per_sector / 1024.0);
}

// Fetch the data from IMAGE file specified  by user
int fetchData(int fd0, char *file1, struct FAT32_INFO *fatInfo, struct Fsinfo *fsInfo, char *function, char destinationPath[]){
    fd0 = open(file1, O_RDONLY);
    if (fd0 < 0){
        printf("Error opening the file\n");
        return -1;
    }
    int fd1 = open(file1, O_RDONLY);
    if (parseBootSector(fd0, fd1, fatInfo, fsInfo, function, destinationPath) == -1){
        return -1;
    }
    return 1;
}

// Parses the boot sector of the FAT32 file system
int parseBootSector(int fd0, int fd1, struct FAT32_INFO *fatInfo, struct Fsinfo *fsInfo, char *function, char destinationPath[]){
    fatInfo = (struct FAT32_INFO *)malloc(sizeof(struct FAT32_INFO));
    fsInfo = (struct Fsinfo *)malloc(sizeof(struct Fsinfo));
    lseek(fd0, 0, SEEK_SET);
    
    for (int i = 0; i < 3; i++){
        read(fd0, &fatInfo->boot_jump[i], 1);
    }
    // parsing the boot sector
    read(fd0, fatInfo->oem_name, 8);
    read(fd0, &fatInfo->bytes_per_sector, 2);
    read(fd0, &fatInfo->sector_per_cluster, 1);
    read(fd0, &fatInfo->reserved_sector_count, 2);
    read(fd0, &fatInfo->num_of_fats, 1);
    read(fd0, &fatInfo->root_entry_count, 2);
    read(fd0, &fatInfo->total_sector16, 2);
    read(fd0, &fatInfo->media, 1);
    read(fd0, &fatInfo->fat_size16, 2);
    read(fd0, &fatInfo->sectors_per_track, 2);
    read(fd0, &fatInfo->num_of_heads, 2);
    read(fd0, &fatInfo->hidden_sectors, 4);
    read(fd0, &fatInfo->total_sector32, 4);
    read(fd0, &fatInfo->fat_size32, 4);
    read(fd0, &fatInfo->extra_flags, 2);
    read(fd0, &fatInfo->version_high, 1);
    read(fd0, &fatInfo->version_number, 1);
    read(fd0, &fatInfo->root_cluster, 4);
    read(fd0, &fatInfo->fs_info, 2);
    read(fd0, &fatInfo->backup_boot_sector, 2);
    read(fd0, &fatInfo->reserved_area, 12);
    read(fd0, &fatInfo->drive_number, 1);
    read(fd0, &fatInfo->reserved_sector, 1);
    read(fd0, &fatInfo->boot_signature, 1);
    read(fd0, &fatInfo->volume_id, 4);
    read(fd0, &fatInfo->volume_label, 11);

    // Compare the function name passed as argument by user and process accordingly
    if (strcmp(function, "info") == 0){
        // Read FSINFO sector
        int infoSector = fatInfo->reserved_sector_count * fatInfo->bytes_per_sector;
        lseek(fd0, infoSector, SEEK_SET);
        read(fd0, &fsInfo->FSI_LeadSig, 4);
        read(fd0, &fsInfo->FSI_Reserved1, 480);
        read(fd0, &fsInfo->FSI_StrucSig, 4);
        read(fd0, &fsInfo->FSI_Free_Count, 4);
        uint32_t fat0 = 0;
        uint32_t fat1 = 0;

        // Calculate the number of data sectors
        int dataSectors = fatInfo->total_sector32 - (fatInfo->reserved_sector_count + (fatInfo->num_of_fats * fatInfo->fat_size32));

        // Calculate the address of the first FAT entry
        int address = fatInfo->bytes_per_sector * fatInfo->reserved_sector_count;
        lseek(fd0, 0, SEEK_SET);
        lseek(fd0, address, SEEK_CUR);
        read(fd0, &fat0, 4);
        read(fd0, &fat1, 4);

        // Loop through FAT entries and count free, bad, and used clusters
        for (int i = 2; i < dataSectors / fatInfo->sector_per_cluster + 2; i++) {
            uint32_t clusterVal = 0;
            read(fd0, &clusterVal, 4);
            if (clusterVal == 0x00000000 || clusterVal == 0x10000000 || clusterVal == 0xF0000000) {
                fatInfo->free_clusters++;
            } else if (clusterVal == BAD_CLUSTER) {
                fatInfo->bad_clusters++;
            } else {
                fatInfo->used_clusters++;
            }
        }
        printInfo(fatInfo);
    }
    else if (strcmp(function, "list") == 0){
        traverseDirectoryTree(fd0, fd1, fatInfo->root_cluster, fatInfo, 0);
    }
    else if (strcmp(function, "get") == 0){
        parseDirectoryPath(fd0, fd1, fatInfo->root_cluster, fatInfo, destinationPath);
    }
    return 1;
}

// Traverse the directory tree recursively starting from specified cluster and then print info about files and directories
void traverseDirectoryTree(int fileDescriptor, int fileSystemDescriptor, uint32_t currentCluster, struct FAT32_INFO *fileSystemInfo, int level){
    uint32_t nextClusterNumber = 0;
    int isEnd = 0;
    int reset = 0;
    int entryCount = 0;

    // Loop until the end of the directory tree is reached
    while (isEnd != 1){
        int sectorNumber = getClusterIndex(currentCluster, fileSystemInfo);
        off_t offset = sectorNumber * fileSystemInfo->bytes_per_sector;
        lseek(fileDescriptor, 0, SEEK_SET);
        lseek(fileDescriptor, offset, SEEK_CUR);

        // Process directory entries in the current cluster
        while (entryCount != (fileSystemInfo->sector_per_cluster * fileSystemInfo->bytes_per_sector) / 32){
            struct DirInfo *dirInfo = (struct DirInfo *)malloc(sizeof(struct DirInfo));
            read(fileDescriptor, &dirInfo->dir_name, 11);
            dirInfo->dir_name[11] = '\0';
            read(fileDescriptor, &dirInfo->dir_attr, 1);
            lseek(fileDescriptor, 1, SEEK_CUR);
            read(fileDescriptor, &dirInfo->dir_crt_time_tenth, 1);
            read(fileDescriptor, &dirInfo->dir_crt_time, 2);
            read(fileDescriptor, &dirInfo->dir_crt_date, 2);
            read(fileDescriptor, &dirInfo->dir_last_access_time, 2);
            read(fileDescriptor, &dirInfo->dir_first_cluster_hi, 2);
            read(fileDescriptor, &dirInfo->dir_wrt_time, 2);
            read(fileDescriptor, &dirInfo->dir_wrt_date, 2);
            read(fileDescriptor, &dirInfo->dir_first_cluster_lo, 2);
            read(fileDescriptor, &dirInfo->dir_file_size, 4);
            entryCount++;
 
            // Check if the end of directory is reached
            if ((int)dirInfo->dir_name[0] == 0x00){
                free(dirInfo);
                nextClusterNumber = getFatEntry(currentCluster, fileSystemDescriptor, fileSystemInfo);
                if (nextClusterNumber >= 0x0ffffff8){
                    isEnd = 1;
                }
                else{
                    level++;
                    traverseDirectoryTree(fileDescriptor, fileSystemDescriptor, nextClusterNumber, fileSystemInfo, level);
                }
                reset = 1;
                break;
            }
            if (dirInfo->dir_name[0] == '.' || (int)dirInfo->dir_name[0] == 0x20){} // skip the single and double dots entries
            else{
                if (dirInfo->dir_name[0] != (char)0xE5){
                    if ((dirInfo->dir_attr & (ATTR_HIDDEN)) || (dirInfo->dir_attr & (ATTR_SYSTEM))){} // skip hidden entries
                    else{
                        if ((dirInfo->dir_attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00){
                            // format to properly print the info removing extra space
                            for (int t = 1; t < level; t++) {
                                printf("-");
                            }
                            printf("File : ");
                            for (int j = 0; j < 8; j++) {
                                if (dirInfo->dir_name[j] == ' ') {
                                    break; 
                                } else {
                                    printf("%c", dirInfo->dir_name[j]);
                                }
                            }
                            int hasExtension = 0;
                            for (int k = 8; k < 11; k++) {
                                if (dirInfo->dir_name[k] != ' ' && dirInfo->dir_name[k] != '~') {
                                    hasExtension = 1;
                                    break;
                                }
                            }
                            if (hasExtension) {
                                printf(".");
                                for (int k = 8; k < 11; k++) {
                                    if (dirInfo->dir_name[k] == ' ' || dirInfo->dir_name[k] == '~') {
                                    } else {
                                        printf("%c", dirInfo->dir_name[k]);
                                    }
                                }
                            }
                            printf("\n");
                        }
                        else if ((dirInfo->dir_attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY){
                            // print the directory info
                            for (int t = 1; t < level; t++){
                                printf("-");
                            }
                            printf("Directory: ");
                            for (int j = 0; j < 12; j++){
                                printf("%c", dirInfo->dir_name[j]);
                            }
                            printf("\n");
                            uint32_t destinationCluster = ((uint32_t)dirInfo->dir_first_cluster_hi << 16) | dirInfo->dir_first_cluster_lo;
                            off_t checkpoint = lseek(fileDescriptor, 0, SEEK_CUR);
                            level++;
                            traverseDirectoryTree(fileDescriptor, fileSystemDescriptor, destinationCluster, fileSystemInfo, level);
                            lseek(fileDescriptor, checkpoint, SEEK_SET);
                        }
                        else if ((dirInfo->dir_attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID){
                            printf("Volume Label: ");
                            for (int j = 0; j < 12; j++){
                                printf("%c", dirInfo->dir_name[j]);
                            }
                            printf("\n");
                        }
                        else{
                            printf("found an invalid directory entry\n");
                        }
                    }
                }
            }
            free(dirInfo);
        }
        if (reset == 0){
            // Get the next cluster number from FAT
            nextClusterNumber = getFatEntry(currentCluster, fileSystemDescriptor, fileSystemInfo);
            if (nextClusterNumber >= 0x0ffffff8){
                isEnd = 1;
                level = 0;

            }
            else{
                currentCluster = nextClusterNumber;
            }
        }
    }
}

// calculate the index of the first sector of a given cluster in the FAT32 FS
int getClusterIndex(int clusterNumber, struct FAT32_INFO *fatInfo){
    int firstDataSector = fatInfo->reserved_sector_count + (fatInfo->num_of_fats * fatInfo->fat_size32);
    int firstSectorOfCluster = ((clusterNumber - 2) * fatInfo->sector_per_cluster) + firstDataSector;
    return firstSectorOfCluster;
}

// retrieves the FAT entry value corresponding to the given cluster number
uint64_t getFatEntry(uint32_t currentCluster, int filePointer, struct FAT32_INFO *fatInfo){
    uint32_t fatEntryValue = 0;
    off_t moveBy = 0;
    moveBy = 4 * currentCluster;
    int address = fatInfo->bytes_per_sector * fatInfo->reserved_sector_count;
    lseek(filePointer, 0, SEEK_SET);
    lseek(filePointer, address, SEEK_CUR);
    lseek(filePointer, moveBy, SEEK_CUR);
    read(filePointer, &fatEntryValue, 4);
    return (fatEntryValue & 0x0FFFFFFF);
}

// traverse and read the content of a file recursively in a FAT32 FS
void traverseAndReadRecursive(int fileDescriptor, int fatPointer, struct DirInfo *directoryInfo, struct FAT32_INFO *fileSystemInfo, uint32_t currentCluster, int fileDescriptorOutput, int remainingBytesToRead){
    int endOfFile = 0;
    int bufferSize = (fileSystemInfo->bytes_per_sector * fileSystemInfo->sector_per_cluster);
    int bytesLeftToRead = remainingBytesToRead;
    char buffer[bufferSize];
    uint32_t nextCluster = 0;
    int sectorNumber = getClusterIndex(currentCluster, fileSystemInfo); // Calculate the sector number and offset for the current cluster
    off_t offset = sectorNumber * fileSystemInfo->bytes_per_sector;
    lseek(fileDescriptor, 0, SEEK_SET);
    lseek(fileDescriptor, offset, SEEK_CUR);

    if (endOfFile == 0){
        // Adjust buffer size if remaining bytes to read is less than the buffer size
        if (bytesLeftToRead < bufferSize){
            bufferSize = bytesLeftToRead;
        }
        ssize_t totalRead = read(fileDescriptor, &buffer, bufferSize);
        buffer[totalRead] = '\0';
        write(fileDescriptorOutput, buffer, totalRead);
        nextCluster = getFatEntry(currentCluster, fatPointer, fileSystemInfo);
        if (nextCluster >= 0x0ffffff8){
            endOfFile = 1;
        }
        // update remaining bytes to read and continue recursion
        else{
            bytesLeftToRead = remainingBytesToRead - bufferSize;
            currentCluster = nextCluster;
            traverseAndReadRecursive(fileDescriptor, fatPointer, directoryInfo, fileSystemInfo, currentCluster, fileDescriptorOutput, bytesLeftToRead);
        }
    }
}

int getDataClusters(int dataFileDescriptor, int fatFileDescriptor, uint32_t currentCluster, struct FAT32_INFO *fileSystemInfo, char *fileNameList[], int fileNameCount) {
    int outputFileDescriptor = open(fileNameList[fileNameCount - 1], O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    int boolFlag = 0;
    int resetFlag = 0;
    int doneFlag = 0;
    int counter = 0;

    // Loop until the end of the file
    while (boolFlag != 1) {
        int sectorNum = getClusterIndex(currentCluster, fileSystemInfo); // Calculate the sector number corresponding to the current cluster
        off_t offset = sectorNum * fileSystemInfo->bytes_per_sector;
        lseek(dataFileDescriptor, 0, SEEK_SET);
        lseek(dataFileDescriptor, offset, SEEK_CUR);
        counter = 0;
        uint32_t nextClusterNum = 0;

        // Iterate through the directory entries within the current cluster
        while (counter != (fileSystemInfo->sector_per_cluster * fileSystemInfo->bytes_per_sector) / 32) {
            struct DirInfo *dirInfo = (struct DirInfo *)malloc(sizeof(struct DirInfo));
            read(dataFileDescriptor, &dirInfo->dir_name, 11);
            dirInfo->dir_name[11] = '\0';
            read(dataFileDescriptor, &dirInfo->dir_attr, 1);
            lseek(dataFileDescriptor, 1, SEEK_CUR);
            read(dataFileDescriptor, &dirInfo->dir_crt_time_tenth, 1);
            read(dataFileDescriptor, &dirInfo->dir_crt_time, 2);
            read(dataFileDescriptor, &dirInfo->dir_crt_date, 2);
            read(dataFileDescriptor, &dirInfo->dir_last_access_time, 2);
            read(dataFileDescriptor, &dirInfo->dir_first_cluster_hi, 2);
            read(dataFileDescriptor, &dirInfo->dir_wrt_time, 2);
            read(dataFileDescriptor, &dirInfo->dir_wrt_date, 2);
            read(dataFileDescriptor, &dirInfo->dir_first_cluster_lo, 2);
            read(dataFileDescriptor, &dirInfo->dir_file_size, 4);
            counter++;
            if ((int)dirInfo->dir_name[0] == 0x00) {
                free(dirInfo);
                nextClusterNum = getFatEntry(currentCluster, fatFileDescriptor, fileSystemInfo);
                // check the end of directory
                if (nextClusterNum >= 0x0ffffff8) {
                    boolFlag = 1;
                }
                // Recursively call the function with the next cluster number
                else {
                    getDataClusters(dataFileDescriptor, fatFileDescriptor, nextClusterNum, fileSystemInfo, fileNameList, fileNameCount);
                }
                resetFlag = 1;
                break;
            }
            // Check if the directory entry represents a file or directory
            if (dirInfo->dir_name[0] == '.' || (int)dirInfo->dir_name[0] == 0x20) {} // skip the single and double dots in entries
            else {
                if (dirInfo->dir_name[0] != (char)0xE5) {
                    if ((dirInfo->dir_attr & (ATTR_HIDDEN)) || (dirInfo->dir_attr & (ATTR_SYSTEM))) {} // skip hidden attributes
                    else {
                        uint32_t destinationCluster = 0;
                        if ((dirInfo->dir_attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
                            char *fileData = dirInfo->dir_name;
                            char *userInput = fileNameList[0];
                            char delimiter = ' ';
                            char delimiter2[] = ".";
                            char *token;
                            int count = 0;
                            int totalWritten = 0;
                            char newFileName[12];
                            char *newUserInput = strtok(userInput, delimiter2);

                            // Extract the file name from directory entry data, removing trailing spaces
                            while (count < 12) {
                                if (*fileData != delimiter) {
                                    newFileName[totalWritten] = *fileData;
                                    totalWritten++;
                                }
                                fileData++;
                                count++;
                            }
                            newFileName[totalWritten] = '\0';
                            // Tokenize and concatenate the remaining parts of the user input
                            while ((token = strtok(NULL, delimiter2)) != NULL) {
                                strcat(newUserInput, token);
                            }
                            if (strcmp(newFileName, newUserInput) == 0) {
                                destinationCluster = ((uint32_t)dirInfo->dir_first_cluster_hi << 16) | dirInfo->dir_first_cluster_lo;
                                traverseAndReadRecursive(dataFileDescriptor, fatFileDescriptor, dirInfo, fileSystemInfo, destinationCluster, outputFileDescriptor, (int)dirInfo->dir_file_size);
                                doneFlag = 1;
                                break;
                            }
                        } 
                        else {
                            // Calculate the destination cluster for the subdirectory
                            destinationCluster = ((uint32_t)dirInfo->dir_first_cluster_hi << 16) | dirInfo->dir_first_cluster_lo;
                            char *userDir = fileNameList[0];
                            char delimiter[] = " ";
                            char *singleChar;
                            char *newFileName = strtok(dirInfo->dir_name, delimiter);
                            while ((singleChar = strtok(NULL, delimiter)) != NULL) {
                                strcat(newFileName, singleChar);
                            }
                            strcat(newFileName, "\0");
                            if (strcmp(newFileName, userDir) == 0) {
                                // Update the file name list and count for recursive traversal
                                for (int i = 1; i < fileNameCount; i++) {
                                    fileNameList[i - 1] = fileNameList[i];
                                }
                                fileNameCount--;
                                off_t checkPoint = lseek(dataFileDescriptor, 0, SEEK_CUR);
                                getDataClusters(dataFileDescriptor, fatFileDescriptor, destinationCluster, fileSystemInfo, fileNameList, fileNameCount);
                                lseek(dataFileDescriptor, checkPoint, SEEK_SET);
                            }
                        }
                    }
                }
            }
            free(dirInfo);
        }
        if (doneFlag == 1) {
            return 1;
        }
        // Check if a recursive call was made
        if (resetFlag == 0) {
            nextClusterNum = getFatEntry(currentCluster, fatFileDescriptor, fileSystemInfo);
            if (nextClusterNum >= 0x0ffffff8) {
                boolFlag = 1;
            } else {
                currentCluster = nextClusterNum; // Update the current cluster with the next cluster number
            }
        }
    }
    return 1;
}

// parses a target directory path and retrieves data clusters associated with the specified directory
void parseDirectoryPath(int fileDescriptor, int fatTable, uint32_t currentCluster, struct FAT32_INFO *filesystemInfo, char targetDirectory[]){
    char *tokenizedPath[MAX];
    int pathComponentCount = 0;
    char *token = strtok(targetDirectory, "/");
    // Loop through the tokenized path
    while (token != NULL){
        if (pathComponentCount == MAX){
            break;
        }
        else{
            tokenizedPath[pathComponentCount] = token;
            pathComponentCount++;
            token = strtok(NULL, "/");
        }
    }
    // retrieve data clusters for the specified directory path
    if (getDataClusters(fileDescriptor, fatTable, currentCluster, filesystemInfo, tokenizedPath, pathComponentCount) == 1){
        printf("\nData retrieval completed successfully...\n");
        printf("\nPlease check --> %s in the same directory\n\n", tokenizedPath[pathComponentCount - 1]);
    }
}
