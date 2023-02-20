# Chip-8

Chip-8 emulator is the *Hello, World!* of the emulator. So it is a good
introduction to this world...

## Description

### Overview
- Display is 64x32, black background and white foreground.
- (0, 0) is located on the top left, (63, 31) at the bottom-right.
- Draws graphics on the screen using *sprites* that is 15 bytes.
  - So a sprite can be 8x15
- Fonts are stores in this format in the reserverd memory space of the
  interpreter. See below for memory layout.
- Several registers
- Rate is 60Hz
- 36 instructions
  - 2 bytes long
  - Most significant first
  - Most significant byte is at even address

### Memory Layout
- 4KB of memory [0x000-0xFFF]
  - [0x000 - 0x200[ are for the interpreter
  - [0x200 - 0xF00[ are for the program
  - [0xF00 - 0xFFF] are reserved for display refresh
  - The entry point is at 0x200

### Registers
- 16 general purpose **8-bits** registers: **V0** .. **VF**
  - **VF** should not be used by program because it is used as flag register
- 1 **16-bits** register **I** used to store memory addresses (only lowest 12 bytes are used)
- 2 **8-bits** specials registers:
  - **DT**: Delay timer
  - **ST**: Sound timer
- 1 **16-bit** program counter register **PC**
- 1 **8-bit** stack register **SP** to store return value when subroutines are called

### Opcode
- Big-endian
- See the [Opcode Table](https://en.wikipedia.org/wiki/CHIP-8#Opcode_table) for list of opcodes.
- The symbols are:
    - *NNN*: address
    - *NN*: 8-bit constant
    - *N*: 4-bit constant
    - *X* and *Y*: 4-bit register identifier
    - *PC* : Program Counter
    - *I* : 16bit register (For memory address) (Similar to void pointer);
    - *VN*: One of the 16 available variables. N may be 0 to F (hexadecimal);

## Implementation

- **C/**: CHIP-8 emulator in C

## Links

- [Chip-8](https://en.wikipedia.org/wiki/CHIP-8)
- [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) 
