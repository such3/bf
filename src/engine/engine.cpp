#include "engine.h"
#include "./../tracers/tracers.h"

void executeCode(MachineState &machine, Options &options,std::unordered_map<int , int> &positions , std::unordered_map<int, int> &revpos){
if (options.verbose)
        {
            std::clog << "Executing the program" << std::endl;
        }

        int cnt = 0;

        unsigned long long steps = 0;
        while (static_cast<size_t>(machine.pc) < machine.program.length())
        {

            if (options.verbose)
            {
                std::clog << "Step Count : " << steps << std::endl;
            }
            if (steps >= options.maxSteps)
            {
                throw RuntimeError("[ERROR] Step limit of " + std::to_string(options.maxSteps) + " reached at index  " + std::to_string(machine.pc) + "\n");
            }
            steps++;

            if (machine.data_ptr == 30000)
            {
                throw RuntimeError("[OVERFLOW] Tape overflow !! You have crossed 30k cells . \n");
            }

            if (options.verbose)
            {
                std::clog << "------------------------------------------------" << std::endl;
                std::clog << "Iteration : " << cnt++ << std::endl;
                std::clog << " char : " << machine.program[machine.pc] << std::endl;
            }

            switch (machine.program[machine.pc])
            {
            case '+':

                traceInstruction(machine.program[machine.pc], options);
                dumpState(machine, options);

                machine.tape[machine.data_ptr]++;
                machine.pc++;

                break;

            case '-':

                traceInstruction(machine.program[machine.pc], options);
                dumpState(machine, options);

                machine.tape[machine.data_ptr]--;
                machine.pc++;

                break;

            case '>':

                traceInstruction(machine.program[machine.pc], options);
                dumpState(machine, options);

                machine.data_ptr++;
                machine.pc++;

                break;

            case '<':

                traceInstruction(machine.program[machine.pc], options);

                dumpState(machine, options);

                if (machine.data_ptr == 0)
                {
                    throw RuntimeError("[ERROR] Access out of tape bounds  trying to access negative  bounds   ");
                }
                machine.data_ptr--;
                machine.pc++;

                break;

            case '.':

                traceInstruction(machine.program[machine.pc], options);
                dumpState(machine, options);

                std::cout << machine.tape[machine.data_ptr];

                machine.pc++;

                break;

            case ',':

                traceInstruction(machine.program[machine.pc], options);
                dumpState(machine, options);

                machine.pc++;

                break;

            case '[':

                traceInstruction(machine.program[machine.pc], options);

                if (machine.tape[machine.data_ptr] == 0)
                {
                    dumpState(machine, options);

                    machine.pc = positions[machine.pc] + 1;
                }
                else
                {
                    dumpState(machine, options);

                    machine.pc++;
                }

                break;

            case ']':

                traceInstruction(machine.program[machine.pc], options);
                if (machine.tape[machine.data_ptr] != 0)
                {
                    dumpState(machine, options);

                    machine.pc = revpos[machine.pc] + 1;
                }
                else
                {
                    dumpState(machine, options);

                    machine.pc++;
                }

                break;

            default:

                dumpState(machine, options);
                if (options.verbose)
                {
                    std::clog << " Skipping other instructions such as new lines and spaces and tabs " << std::endl;
                }

                machine.pc++;
            }
            if (options.verbose)
            {
                std::clog << "------------------------------------------------" << std::endl;
            }
        }

        if (options.verbose)
        {
            std::clog << "\n Final Dump state " << std::endl;
            dumpState(machine, options);
        }
}
