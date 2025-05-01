# chip8emulator
been putting this off for almost 4 years, finally taking a crack at it                                                                           
## Instruction Implementation Progress:

- [x] 00E0 – CLS: Clear the display
- [x] 00EE – RET: Return from subroutine
- [ ] 0nnn – SYS addr: Jump to a machine code routine at nnn (ignored in most interpreters)
- [x] 1nnn – JP addr: Jump to location nnn
- [x] 2nnn – CALL addr: Call subroutine at nnn
- [x] 3xkk – SE Vx, byte: Skip next instruction if Vx == kk
- [x] 4xkk – SNE Vx, byte: Skip next instruction if Vx != kk
- [x] 5xy0 – SE Vx, Vy: Skip next instruction if Vx == Vy
- [x] 6xkk – LD Vx, byte: Set Vx = kk
- [x] 7xkk – ADD Vx, byte: Set Vx = Vx + kk
- [x] 8xy0 – LD Vx, Vy: Set Vx = Vy
- [x] 8xy1 – OR Vx, Vy: Set Vx = Vx OR Vy
- [x] 8xy2 – AND Vx, Vy: Set Vx = Vx AND Vy
- [x] 8xy3 – XOR Vx, Vy: Set Vx = Vx XOR Vy
- [x] 8xy4 – ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry
- [x] 8xy5 – SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow
- [x] 8xy6 – SHR Vx {, Vy}: Set Vx = Vx SHR 1 (legacy: Vy into Vx, then shift)
- [x] 8xy7 – SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow
- [x] 8xyE – SHL Vx {, Vy}: Set Vx = Vx SHL 1 (legacy: Vy into Vx, then shift)
- [x] 9xy0 – SNE Vx, Vy: Skip next instruction if Vx != Vy
- [x] Annn – LD I, addr: Set I = nnn
- [x] Bnnn – JP V0, addr: Jump to location nnn + V0
- [x] Cxkk – RND Vx, byte: Set Vx = random byte AND kk
- [ ] Dxyn – DRW Vx, Vy, nibble: Display n-byte sprite at (Vx, Vy), set VF = collision
- [x] Ex9E – SKP Vx: Skip next instruction if key with value of Vx is pressed
- [x] ExA1 – SKNP Vx: Skip next instruction if key with value of Vx is not pressed
- [x] Fx07 – LD Vx, DT: Set Vx = delay timer value
- [x] Fx0A – LD Vx, K: Wait for a key press, store the value in Vx
- [x] Fx15 – LD DT, Vx: Set delay timer = Vx
- [x] Fx18 – LD ST, Vx: Set sound timer = Vx
- [x] Fx1E – ADD I, Vx: Set I = I + Vx
- [x] Fx29 – LD F, Vx: Set I = location of sprite for digit Vx
- [ ] Fx33 – LD B, Vx: Store BCD representation of Vx in memory at I, I+1, I+2
- [ ] Fx55 – LD [I], Vx: Store registers V0 through Vx in memory starting at I
- [ ] Fx65 – LD Vx, [I]: Read registers V0 through Vx from memory starting at I

