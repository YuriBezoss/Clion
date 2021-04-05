//
// Created by Ocean on 4/4/2021.
//

#include <algorithm>
#include <cstdint>
#include "my_startup.h"

namespace mcal
{
    namespace cpu
    {
        void init() { /* simulated initialization of the CPU */ }
    } // namespace mcal::cpu

    namespace wdg
    {
        struct secure
        {
            static void trigger() { /* simulated watchdog trigger */ }
        };
    } // namespace mcal::wdg
}

namespace crt {
    //Linker-defined begin and end of the .bss section.
    extern std::uintptr_t _bss_begin;
    extern std::uintptr_t _bss_end;

    void init_bss() {
        //Clear the bss segment.
        std::fill(&_bss_begin, &_bss_end, 0U);
    }

    //Linker-defined begin of rodata.
    extern std::uintptr_t _rodata_begin;

    //Linker-defined begin and end of data.
    extern std::uintptr_t _data_begin;
    extern std::uintptr_t _data_end;

    void init_data() {
        //Calculate the size of the data section
        const std::size_t cnt = (&_data_end - &_data_begin);

        //Copy the rohdata section to the data section.
        std::copy(&_rodata_begin, &_rodata_begin + cnt, &_data_begin);
    }

    typedef void (*function_type)();

    //Linker-defined begin and end of the ctors section.
    extern function_type _ctor_begin[];
    extern function_type _ctor_end[];

    void init_ctors() {
        //Clear the bss segment.
        std::for_each(_ctors_begin, _ctors_end, [](const function_type pf){ pf(); });
    }

    void init_ram  () {
        //Clear the bss segment.
        std::fill(&_bss_begin, &_bss_end, 0U);
    }
    void init_ctors() { /* simulated initialization of static constructors */}

} // namespace crt

extern "C"
void simulated_main(void)
{
    // This is a sinulated main() subroutine.
    for(;;)
    {
        std::cout << "in simulated main()" << std::endl;

        mcal::wdg::secure::trigger();
    }
}

void __my_startup()
{
    //Load the sreg register
    asm volatile("eor r1, r1");
    asm volatile("out 0x3f, r1");

    //Set the stack pointer
    asm volatile("ldi r28, lo8(__initial_stack_pointer)");
    asm volatile("ldi r29, hi8(__initial_stack_pointer)");

    //Load the sph register (stack pointer high).
    asm volatile("out 0x3e, r29");

    //Load the spl register (stack pointer low).
    asm volatile("out 0x3d, r28");

    // CPU Initialization, including watchdog,
    // port, oscillators (i.e. clocks).
    mcal::cpu::init();

    // Initialize statics from ROM to RAM.
    // Zero-clear default-initialized static RAM.
    crt::init_ram();
    mcal::wdg::secure::trigger();

    // Call all constructor initializations.
    crt::init_ctors();
    mcal::wdg::secure::trigger();

    // Call the simulated main (and never return).
    //simulated_main();
    asm volatile("call simulated_main");

    // Catch an unexpected return from main.
    for(;;)
    {
        // Replace with a loud error if desired.
        mcal::wdg::secure::trigger();
    }
}