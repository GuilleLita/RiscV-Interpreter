# RiscV-interpeter 

This RISCV interpreter is a simple rv32i interpreter written in C++ for teaching purposes. 

This project has been developed within the framework of the subject “Garantia y Seguridad” of Computer Engineering in Zaragoza. This project is based on the original peRISCVcope project.[peRISCVcope: A Tiny Teaching-Oriented RISC-V
Interpreter](https://www.doi.org/10.1109/DCIS55711.2022.9970050).[peRRISCVcope Github page](https://github.com/dariosg/periscvcope)


## Cross-Compiler

    git clone https://github.com/riscv/riscv-gnu-toolchain.git
    cd riscv-gnu-toolchain
    ./configure --prefix=$HOME/usr/riscv/  --with-arch=rv32g --with-abi=ilp32d
    make

## References

* Good [ELF](https://www.ics.uci.edu/~aburtsev/238P/hw/hw3-elf/hw3-elf.html) explanations
* [RISC-V ELF](https://github.com/riscv-non-isa/riscv-elf-psabi-doc/)
