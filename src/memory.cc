#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>

#include <memory.hh>

using namespace mem;

void
memory::load_binary(const std::string& binfile)
{
    using ifile_iter = std::istream_iterator<uint8_t>;

    std::ifstream ifile(binfile, std::ios::binary);

    if (ifile.is_open() == false) {
	std::cerr << "Unable to open "<< binfile << std::endl;
	std::exit(EXIT_FAILURE);
    }

    // copy the binary into memory
    // Stop eating new lines in binary mode!!!
    ifile.unsetf(std::ios::skipws);

    ifile.seekg(0, std::ios::end);
    const auto ifsize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    _binary.reserve(ifsize);
    _binary.insert(_binary.begin(),
	    ifile_iter(ifile),
	    ifile_iter());
    ifile.close();


    // read elf header
    std::memcpy(&_ehdr, &_binary[0], sizeof(_ehdr));

    //ensure elf file
    if(_ehdr.e_ident[0] != ELFMAG0 || _ehdr.e_ident[1] != ELFMAG1 || _ehdr.e_ident[2] != ELFMAG2 || _ehdr.e_ident[3] != ELFMAG3) {
        std::cerr << "Not an ELF Binary" << std::endl;
	    std::exit(EXIT_FAILURE);
    }

    // ensure riscv32
    if(_ehdr.e_machine != EM_RISCV || _ehdr.e_ident[4] != ELFCLASS32) {
        std::cerr << "Binary is not a RISC-V 32 executable" << std::endl;
	    std::exit(EXIT_FAILURE);
    }

    // ensure the binary has a correct program table
    if(_ehdr.e_phnum <= 0) {
        std::cerr << "Binary has no correct program table" << std::endl;
	    std::exit(EXIT_FAILURE);
    }

    // entry point
    /* if(_ehdr.e_entry == 0){
        std::cerr << "Binary has no entry point" << std::endl;
	    std::exit(EXIT_FAILURE);
    } */

    // load sections in memory
    // read ELF program header table,

    _phdr.resize(_ehdr.e_phnum);
    std::memcpy(&_phdr[0], &_binary[_ehdr.e_phoff], _ehdr.e_phnum * sizeof(Elf32_Phdr));

    for(auto& ph : _phdr) {
        if(ph.p_type == PT_LOAD) {
            segment seg(ph.p_vaddr, ph.p_memsz);
            std::memcpy(&seg._content[0], &_binary[ph.p_offset], ph.p_filesz);
            _segments.push_back(seg);
        }
    }

    // ... to be completed


}


void memory::dump_hex(const ssize_t segmentiden)
{
    const segment seg = _segments[segmentiden];

    std::cout << "Segment " << segmentiden << ":\n";
    std::cout << "Initial Address: 0x" << std::hex << seg._initial_address << std::endl;
    std::cout << "Size: " << std::dec << seg._content.size() << std::endl;
    std::cout << "Content:\n";
    
    for(size_t i = 0; i < seg._content.size(); i++) {
        std::cout << std::hex << seg._content[i];
        if((i+1) % 8 == 0) {
            std::cout << std::endl;
        }
    }
}
