#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#define Version "0.0.1"


//from stdlib.h but we dont import bc of div();
extern void exit(int ec);
extern void free(void* p);
extern void* malloc(size_t size);


FILE* fin;


struct {
    unsigned long long main_offset;
    unsigned long long data_offset;
    unsigned long long rodata_offset;
}__attribute__((packed)) Header = {0, 0, 0};


unsigned long long R[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned char* Code = NULL;
unsigned long long CodeSize = 0;
unsigned char* Data = NULL;
unsigned long long DataSize = 0;
unsigned char* ROData = NULL;
unsigned long long RODataSize = 0;
unsigned char* stack = NULL;

unsigned char opcode = 0;

enum {
    true = 0, 
    false = 1
} running = true;


void close(int rc) {
    if (fin != NULL) fclose(fin);

    if (Code != NULL) free(Code);
    if (Data != NULL) free(Data);
    if (ROData != NULL) free(ROData);

    exit(rc);
}



//VMFC
void xtn_vmfc() {
    switch (((unsigned long long*)(Code + R[15] + 3))[0]) {
        case 0x00:
            running = false;
            break;
        case 0x01:
            printf("!%c", (char)R[0]);
            break;
        default:
            printf("\n!Unknown vmfc!\n");
            close(1);
    }
}

typedef struct {
    unsigned long long* dest;
    unsigned long long* src;
} reg_operand_packet;

reg_operand_packet getreg(unsigned char in) {
    reg_operand_packet ret;

    ret.dest = &R[(in & 0xf0) >> 4];
    ret.src = &R[in & 0x0f];

    return ret;
}



//Core



void mul() {
    reg_operand_packet regs = getreg(Code[R[15] + 1]);
    //add memory and size checks

    switch ((Code[R[15]] & 0b00110000) >> 4) {
        case 0x00:
            regs.dest[0] *= regs.src[0];
    }

    
    R[15] += 2;
}

void div() {
    
}

void mod() {
    
}

void add() {
    
}

void sub() {
    
}


void or() {
    
}

void and() {
    
}

void xor() {
    
}

void not() {
    
}


void mov() {
    
}

void set() {
    
}


void push() {
    
}

void pop() {
    
}


void cmp() {
    
}

void cmov() {
    
}


void xtn() {
    switch (Code[R[15] + 2]) {
        case 0x00:
            if (Code[R[15] + 1] > 64) {
                printf("Extended operaton to large\n");
                close(1);
            }
            xtn_vmfc();
            R[15] =+ Code[R[15]+1];
            break;
        default:
            printf("\n!Unrecognized extended operatore!\n");
            close(1);
    }
}








int run() {
    while (0 == 0) {
        if (running == false) return 0;
        
        if (R[15] > (CodeSize - 1)) {
            printf("\n\n!!!Program Tried to run out of code-area!!!\n\n");
            close(1);
        }


        switch (Code[R[15]] & 0x0f) {
            case 0x0:
                mul();
                break;
            case 0x1:
                div();
                break;
            case 0x2:
                mod();
                break;
            case 0x3:
                add();
                break;
            case 0x4:
                sub();
                break;
            case 0x5:
                or();
                break;
            case 0x6:
                and();
                break;
            case 0x7:
                xor();
                break;
            case 0x8:
                not();
                break;
            case 0x9:
                mov();
                break;
            case 0xa:
                set();
                break;
            case 0xb:
                push();
                break;
            case 0xc:
                pop();
                break;
            case 0xd:
                cmp();
                break;
            case 0xe:
                cmov();
                break;
            case 0xf:
                xtn();
                break;
        }
    }
}





void init() {
    unsigned long long size = 0;

    fread(&Header, sizeof(Header), 1, fin);

    Code = malloc(Header.data_offset);
    fread(Code, Header.data_offset, 1, fin);


    if (Header.data_offset != 0) {
        if (Header.rodata_offset != 0) {
            size = Header.rodata_offset - Header.data_offset;
        }
        else {
            fseek(fin, 0L, SEEK_END);
            size = ftell(fin) - sizeof(Header);
            rewind(fin);
            size -= Header.data_offset;
        }
        if (CodeSize == 0) CodeSize = Header.data_offset;
        Data = malloc(size);
        fseek(fin, Header.data_offset + sizeof(Header), SEEK_SET);
        fread(Data, size, 1, fin);
        DataSize = size;
    }

    if (Header.rodata_offset != 0) {
        fseek(fin, 0L, SEEK_END);
        size = ftell(fin) - sizeof(Header);
        rewind(fin);

        size -= Header.rodata_offset;

        if (CodeSize == 0) CodeSize = Header.rodata_offset;

        ROData = malloc(size);
        fseek(fin, Header.rodata_offset + sizeof(Header), SEEK_SET);
        fread(ROData, (size), 1, fin);
        RODataSize = size;
    }

    if (CodeSize == 0) {
        fseek(fin, 0L, SEEK_END);
        CodeSize = ftell(fin) - sizeof(Header);
        rewind(fin);
    }

    stack = malloc(sizeof(unsigned char) * (1024*8));

    R[15] = Header.main_offset;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("No arguments\n");
        return 1;
    }

    fin = fopen(argv[1], "rb");
    if (fin == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    printf("\nRunning: %s in EthercyVM version %s\n\n", argv[1], Version);

    init();
    run();


    printf("\n\n\nExited with return code: 0x%llx\n", R[0]);
    return 0;
}