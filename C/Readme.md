# Steps

Steps are not yet well defined but we will:
- [ ] Manage the [display](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#display)
  - [ ] using SDL
  - [ ] implementing draw command
- [x] Fetch insn
- [x] Decode insn
- [ ] Execute insn

# Build

- Just run `make`

# Testing

- For testing we will try to print the IBM logo
- Currently without execution and interction with SDL we are just looping through opcodes but we see that decoding is working...
```
➜ ./chip8-emulator IBM_logo.ch8 
PC[0x0200] | insn: 0x00e0: Clear screen not implemented
PC[0x0202] | insn: 0xa22a: Set I to 0x022a not implemented
PC[0x0204] | insn: 0x600c: Set V0 to 0x000c Opcode not implemented
PC[0x0206] | insn: 0x6108: Set V1 to 0x0008 Opcode not implemented
PC[0x0208] | insn: 0xd01f: Draw at (V0, V1, height 15) not implemented
PC[0x020a] | insn: 0x7009: Adds 0x0009 to V0 not implemented
PC[0x020c] | insn: 0xa239: Set I to 0x0239 not implemented
PC[0x020e] | insn: 0xd01f: Draw at (V0, V1, height 15) not implemented
PC[0x0210] | insn: 0xa248: Set I to 0x0248 not implemented
PC[0x0212] | insn: 0x7008: Adds 0x0008 to V0 not implemented
PC[0x0214] | insn: 0xd01f: Draw at (V0, V1, height 15) not implemented
PC[0x0216] | insn: 0x7004: Adds 0x0004 to V0 not implemented
PC[0x0218] | insn: 0xa257: Set I to 0x0257 not implemented
PC[0x021a] | insn: 0xd01f: Draw at (V0, V1, height 15) not implemented
PC[0x021c] | insn: 0x7008: Adds 0x0008 to V0 not implemented
PC[0x021e] | insn: 0xa266: Set I to 0x0266 not implemented
PC[0x0220] | insn: 0xd01f: Draw at (V0, V1, height 15) not implemented
PC[0x0222] | insn: 0x7008: Adds 0x0008 to V0 not implemented
PC[0x0224] | insn: 0xa275: Set I to 0x0275 not implemented
PC[0x0226] | insn: 0xd01f: Draw at (V0, V1, height 15) not implemented
PC[0x0228] | insn: 0x1228: Jump to 0x0228 not implemented
...
```
