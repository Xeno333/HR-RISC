#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#define Version "0.0.1"


//from stdlib.h but we dont import bc of div();
extern void exit(int ec);
extern void free(void* p);
extern void* malloc(size_t size);


FILE* fin;


unsigned long long R[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned char* Memory = NULL;
unsigned long long MemorySize = 0;

enum {
    true = 0, 
    false = 1
} running = true;

typedef enum {
    REG = 0, 
    MEM = 1
} Optype;

typedef struct {
    unsigned char log_2_size : 2;
    Optype dst_type : 1;
    Optype src_type : 1;
    unsigned char opc : 4;
}__attribute__((packed)) opcode;
    //opcode op;
    //op.log_2_size = (Memory[R[15]] >> 6) & 0x03;
    //op.dst_type = (Memory[R[15]] >> 5) & 0x01;
    //op.src_type = (Memory[R[15]] >> 4) & 0x01;
    //op.opc = Memory[R[15]] & 0x0F;   


void close(int rc) {
    if (fin != NULL) fclose(fin);

    if (Memory != NULL) free(Memory);

    printf("\n\nExited with return code: 0x%llx\n", R[0]);

    exit(rc);
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



//Fun
unsigned long long power(unsigned long long base, int exponent) {
    unsigned long long result = 1.0;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

unsigned long long get_size_mask(int  e) {
    unsigned long long mask = ~0;
    for (int i = power(2, e); i != 0; i--) {
        mask = mask << 8;
    }
    return ~mask;
}


unsigned long long get_mem_q(unsigned long long p) {
    unsigned long long res = Memory[p];
    for (int i = 1; i != 8; i++) {
        res = res | (Memory[p + i]<< (8 * i));
    }
    return res;
}

void set_mem_q(unsigned long long plc, unsigned long long p) {
    for (int i = 0; i != 8; i++) {
        Memory[plc + i] = (p >> (8 * i)) & 0xff;
    }
}

//VMFC
void xtn_vmfc() {
    switch (((unsigned long long*)(Memory + R[15] + 3))[0]) {
        case 0x00:
            running = false;
            break;
        case 0x01:
            printf("%c", (char)R[0]);
            break;
        default:
            printf("\n!Unknown vmfc!\n");
            close(1);
    }
}
//Core



void mul() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 * i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void div() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 / i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void mod() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 % i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void add() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 + i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void sub() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 - i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}


void or() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 | i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void and() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 & i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void xor() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1, i2;
    unsigned long long v;

    //in 1
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }
    //in 2
    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i1 ^ i2;//operate
    
    //out
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void not() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i1;
    unsigned long long v;

    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        i1 = get_mem_q(*regs.dest) & size_mask;
    }
    else {
        i1 = *regs.dest & size_mask;
    }

    v = ~i1;//operate
    
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}


void mov() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long i2;
    unsigned long long v;

    if ((Memory[R[15]] >> 4) & 0x01 == 1) {
        i2 = get_mem_q(*regs.src) & size_mask;
    }
    else {
        i2 = *regs.src & size_mask;
    }

    v = i2;//operate
    
    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, v & size_mask);
    }
    else {
        *regs.dest = v;
    }

    if (regs.dest != & R[15])
        R[15] += 2;
}

void set() {
    reg_operand_packet regs = getreg(Memory[R[15] + 1]);
    unsigned long long size_mask = get_size_mask((Memory[R[15]] >> 6) & 0x03);
    unsigned long long rawin = get_mem_q(R[15] + 2);

    if ((Memory[R[15]] >> 5) & 0x01 == 1) {
        set_mem_q(*regs.dest, rawin & size_mask);
    }
    else {
        *regs.dest = rawin;
    }

    if (regs.dest != & R[15])
        R[15] += 2 + power(2, Memory[R[15]] >> 6);
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
    switch (Memory[R[15] + 2]) {
        case 0x00:
            if (Memory[R[15] + 1] > 64) {
                printf("Extended operaton to large\n");
                close(1);
            }
            xtn_vmfc();
            break;
        default:
            printf("\n!Unrecognized extended operatore!\n");
            close(1);
    }
    R[15] += Memory[R[15]+1];
}








int run() {
    while (0 == 0) {
        if (running == false) return 0;
        
        if (R[15] > (MemorySize - 1)) {
            printf("\n\n!!!Program Tried to run out of Memory!!!\n\n");
            close(1);
        }


        switch (Memory[R[15]] & 0x0f) {
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

    rewind(fin);
    fseek(fin, 0L, SEEK_END);
    MemorySize = ftell(fin);
    rewind(fin);

    Memory = malloc(MemorySize);
    fread(Memory, MemorySize, 1, fin);

    R[15] = 0;
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

    close(0);
}