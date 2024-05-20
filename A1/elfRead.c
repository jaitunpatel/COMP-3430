
//Included some required header files
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#pragma pack(push)
#pragma pack(1)
//struct that stores the data from elf_header
typedef struct ELF_HEADER{
    uint8_t bit_format;        // 32 or 64 bit
    uint8_t endianness;        //little endian
    uint8_t OS;            //operating system
    uint16_t obj_file_type;    //type of the object file
    uint16_t ISA;              // instruction set architecture
    uint64_t entry_point64;    // entry address of 64 bit
    uint64_t program_header64; // starting point for program header
    uint64_t section_header64; // starting point for section header
    uint16_t program_header_size;     // size of program_header
    uint16_t num_program_header; // number of program header
    uint16_t section_header_size;     // size section header
    uint16_t total_section_header; // total section header
    uint16_t section_header_index;    // entry address in the section header
} elf_header;

//strcut to store the values for program_header values
typedef struct Program_Header{
    uint32_t seg_type;   // type of segmentation
    uint64_t program_offset64; // offset to the binary data in the file
    uint64_t virtual_address64; // virtual address
    uint64_t image_size64; // size of the image in the file
    unsigned char value; // read the byte data of the header (one by one)
} program_header;

//strcut to store the values of the section header
typedef struct Section_Header{
    char name[10]; // name of the header
    uint32_t offset_name; // offset to the name
    uint32_t type; // type of header
    uint64_t virtual_address64; // virtual address of the header
    uint64_t string_table_offset64; // offset to the string table in the section header(at the end of all headers)
    uint64_t section_offset64; //offset to the binary data in the file
    uint64_t image_size64; // size of the image in the file
    unsigned char data; // to store the byte data of the header
} section_header;
#pragma pack(pop)

// method declarations
void elfReader(elf_header *header, int handle_elf);
void printElfData(elf_header *header);
void program_reader(program_header *prog_header, elf_header *e_header, int handle_elf);
void section_reader(section_header *section_header, elf_header *e_header, int handle_elf);

//main function - opens the file and calls various methods to extract data
int main(int argc, char *argv[]){
    if (argc > 1){ // checks the arguments must be more than 1    
        char *fileName = argv[1];                            // name of the file from the argument
        elf_header fileHeader;                               // elf_header struct
        program_header prog_header;                          // program_header struct
        section_header section_header;                       //section_header struct
        int return_value = open(fileName, O_RDONLY);          //opens the file - only for reading
        elfReader(&fileHeader, return_value);                 // calling elf reader
        printElfData(&fileHeader);                           //printing elf header
        program_reader(&prog_header, &fileHeader, return_value); //storing and printing program_header values
        section_reader(&section_header, &fileHeader, return_value); //stroing and printing section_header values
    }
    return 0;
}

//reads the elf_header and stores the values into the struct
void elfReader(elf_header *header, int handle_elf){
    lseek(handle_elf, 4, SEEK_CUR);             //skipping 4 bytes from the curr offset
    read(handle_elf, &header->bit_format, 1);   //reading bit-format
    read(handle_elf, &header->endianness, 1);   //reading endianness
    lseek(handle_elf, 2, SEEK_CUR);             //skipping 2 bytes
    read(handle_elf, &header->OS, 1);           //version of operating system
    lseek(handle_elf, 7, SEEK_CUR);             //skipping 7 bytes
    read(handle_elf, &header->obj_file_type, 2);//reading file type
    read(handle_elf, &header->ISA, 2);          //reading ISA type
    lseek(handle_elf, 4, SEEK_CUR);             //skipping 4 bytes

    // reading various things specific to 32 bit format
    if (header->bit_format == 1){
        printf("\n The format type byte is a 32-bit format \n");
        return;
    } 
    else { // reading things for 64 bit format
        read(handle_elf, &header->entry_point64, 8);
        read(handle_elf, &header->program_header64, 8);
        read(handle_elf, &header->section_header64, 8);
    } 
    lseek(handle_elf, 6, SEEK_CUR); // skipping 6 bytes

    //reading header size and string table offset
    read(handle_elf, &header->program_header_size, 2);
    read(handle_elf, &header->num_program_header, 2);
    read(handle_elf, &header->section_header_size, 2);
    read(handle_elf, &header->total_section_header, 2);
    read(handle_elf, &header->section_header_index, 2);
}

//print all the data of the elf_header
//reference to the elf_header struct
void printElfData(elf_header *header){
    printf("\nElf Header: \n");

    if (header->bit_format == 2){ //print bit-format
        printf("* 64-bit\n");
    }
    else{
        printf("* 32-bit\n");
    }

    //endianness
    if (header->endianness == 1){
        printf("* little endian\n");
    }
    else{
        printf("* big endian\n");
    }

    //ABI operating system
    printf("* compiled for");
    printf(" 0x%.2x", header->OS);
    printf(" (operating system)\n");

    //printing obj file type
    printf("* has type");
    printf(" 0x%.2x\n", header->obj_file_type);

    //printing ISA operating system
    printf("* compiled for");
    printf(" 0x%.2x", header->ISA);
    printf(" (isa)\n");

    //printing header entry points
    printf("* entry point address ");
    if (header->bit_format == 1){
        //printf("0x%.16x\n", header->entry_point32);
        return;
    }
    else{
        printf("0x%.16lx\n", header->entry_point64);
    }

    //printing program header entry address
    printf("* program header table starts at ");
    if (header->bit_format == 1){
        return;
    }
    else{
        printf("0x%.16lx\n", header->program_header64);
    }
    printf("* there are %d program headers , each is %d bytes\n", header->num_program_header, header->program_header_size);
    printf("* there are %d section headers , each is %d bytes\n", header->total_section_header, header->section_header_size);
    printf("* the section header string table is %d\n", header->section_header_index);
}

//input : pointers of program header and section header
void program_reader(program_header *prog_header, elf_header *e_header, int handle_elf){
    // Loop through all the program_header of the file
    for (int i = 0; i < e_header->num_program_header; i++){
        //if executable has 32 bit format data
        if (e_header->bit_format == 1){
            return;
        }
        else {
            printf("\n");
            // go to the start of the program header
            lseek(handle_elf, e_header->program_header64 + (i * e_header->program_header_size), SEEK_SET);

            //read all the required data and store it in the struct
            read(handle_elf, &prog_header->seg_type, 4);
            lseek(handle_elf, 4, SEEK_CUR);
            read(handle_elf, &prog_header->program_offset64, 8);
            read(handle_elf, &prog_header->virtual_address64, 8);
            lseek(handle_elf, 8, SEEK_CUR);
            read(handle_elf, &prog_header->image_size64, 8);

            //printing the fields
            printf("Program Header #%d: \n", i);
            printf("* segment type ");
            printf("0x%.8x\n", prog_header->seg_type);
            printf("* virtual address of the segment ");
            printf("0x%.16lx\n", prog_header->virtual_address64);
            printf("* size in the file %lu bytes\n", prog_header->image_size64);
            printf("* first upto 32 bytes starting at 0x%.16lx\n ", prog_header->program_offset64);

            lseek(handle_elf, prog_header->program_offset64, SEEK_SET);
            for (int i = 1; i <= 32; i++){
                read(handle_elf, &prog_header->value, 1);
                if (i != 16 && i != 32){
                    printf("%02x ", prog_header->value);
                }
                else{
                    printf("%02x \n", prog_header->value);
                }
            }
        }
    }
}

//reading and printing all the required details of section header
//input : reference to the section header and elf_header(to get the starting address of the section_header)
//      : file handle_elfr
void section_reader(section_header *section_header, elf_header *e_header, int handle_elf){
    // loop through all available section headers
    for (int i = 0; i < e_header->total_section_header; i++){
        if (e_header->bit_format == 1){
            return;
        }
        else { // stuff on 64 bit file data
            printf(" \n ");
            lseek(handle_elf, e_header->section_header64 + (i * e_header->section_header_size), SEEK_SET);
            read(handle_elf, &section_header->offset_name, 4);
            lseek(handle_elf, e_header->section_header64 + (e_header->section_header_index * e_header->section_header_size) + 24, SEEK_SET);
            read(handle_elf, &section_header->string_table_offset64, 8);
            lseek(handle_elf, section_header->offset_name + section_header->string_table_offset64, SEEK_SET);
            read(handle_elf, &section_header->name, 10);
            lseek(handle_elf, e_header->section_header64 + (i * e_header->section_header_size) + 4, SEEK_SET);
            read(handle_elf, &section_header->type, 4);
            lseek(handle_elf, 8, SEEK_CUR);

            read(handle_elf, &section_header->virtual_address64, 8);
            read(handle_elf, &section_header->section_offset64, 8);
            read(handle_elf, &section_header->image_size64, 8);

            printf("Section header #%d: \n", i);
            printf("* section name >>%s<<\n", section_header->name);
            printf("* type 0x%.2x\n", section_header->type);
            printf("* virtual address of section 0x%.16lx\n", section_header->virtual_address64);
            printf("* size in file %lu bytes\n", section_header->image_size64);
            printf("* first up to 32 bytes starting at file offset 0x%.16lx\n", section_header->section_offset64);

            lseek(handle_elf, section_header->section_offset64, SEEK_SET);
            for (int i = 1; i <= 32; i++){
                read(handle_elf, &section_header->data, 1);
                printf("%02x ", htole32(section_header->data));
                if (i == 16 || i == 32)
                    printf("\n");
            }
        }
    }
}



