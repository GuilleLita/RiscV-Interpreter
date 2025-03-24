#include <iostream>
#include <map>

#include <instructions.hh>
#include <memory.hh>
#include <processor.hh>

using namespace instrs;
using namespace mem;


int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Invalid Syntax: peRISCVcope <program>" << std::endl;
        exit(1);
    }

   memory mem;
   processor proc;

   std::cout << "Loading program: " << argv[1] << std::endl;


   mem.load_binary(argv[1]);

    //Tabla dispatc
   std::map<uint32_t,instr_emulation> dispatch = {
    {0b0000011, instrs::load},
    {0b0100011, instrs::store},
    {0b0010011, instrs::arithmetic_i},
    {0b0110011, instrs::arithmetic_r},
    {0b1101111, instrs::jal},
    {0b1100011, instrs::branch},
    {0b0110111, instrs::lui},
    {0b1100111, instrs::jalr},
    //Instrucciones
   };

   // read the entry point

   proc.write_pc(mem.entry_point());

   // set the stack pointer
   // the stack grows downward with the stack pointer always being 16-byte aligned
   proc.write_reg(processor::sp, memory::stack_top);

    address_t pc = mem.entry_point(), next_pc{0x0}  ;

   size_t exec_instrs = 0;
   do
   {
        pc = proc.read_pc();
        //fetch the instruction
        uint32_t raw_inst = mem.read<uint32_t>(pc);
        // decode the instruction
        instruction ints{raw_inst};

        // execute the instruction
        next_pc = dispatch[ints.opcode()](mem, proc, raw_inst);

        exec_instrs++;
   } while (next_pc != pc); // look for while(1) in the code

   std::cout << "Number of executed instructions: " << exec_instrs << std::endl;


    
   if(std::string(argv[1]) == "../examples/factorial"){
        uint32_t a0_result = proc.read_reg(10); // Leer el registro a0 
        std::cout << "Resultado de factorial de 5(120): " << std::dec << a0_result << std::endl;
       
   }else if(std::string(argv[1]) == "../examples/add_array"){
        // Mostrar el resultado de la suma del array (pila s0 - 20)
        uint32_t s0 = proc.read_reg(8); // Leer el registro s0
        address_t result_addr = s0 - 20; // Dirección de la variable local que almacena el resultado
        uint32_t stack_result = mem.read<uint32_t>(result_addr); // Leer el resultado de la pila
        std::cout << "Resultado de add_array(6): " << std::dec << stack_result << std::endl;
   }
   
   
    return 0;
}
