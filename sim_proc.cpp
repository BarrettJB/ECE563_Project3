#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_proc.h"
#include "pipeline.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
 */
int main (int argc, char* argv[])
{
	FILE *FP;               // File handler
	char *trace_file;       // Variable that holds trace file name;
	proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
	int op_type, dest, src1, src2;  // Variables are read from trace file
	unsigned long int pc; // Variable holds the pc read from input file


    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    /*printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
            */
    // Open trace_file in read mode
	FP = fopen(trace_file, "r");

	//FP = fopen("test_trace.txt", "r");
	if(FP == NULL)
	{
		// Throw error and exit if fopen() failed
		printf("Error: Unable to open file %s\n", trace_file);
		exit(EXIT_FAILURE);
	}


	//Create the pipeline
	instr* input = new instr[params.width];
	Pipeline pl(params.width, params.iq_size, params.rob_size);
	//printf("creating objects...\n");
	//params.width = 1;
	//instr* input = new instr[params.width];
	//Pipeline pl(params.width, 8, 16);
	int traceLine = 0;
	//Run until the pipeline has finished
	while(!pl.finished)
	{
		//if file is empty or proccessor is stalled we need to send empty inputs
		for (int i = 0; i < params.width; i++) {
			//On stall don't update instruction
			if(pl.isStalled()) {
				//printf("needed to stall DI: %d  RN: %d cycle: %d\n",pl.mDIStall,pl.mRNStall,pl.cycle);
			}
			else
			{
				if(pl.eof) {
					input = new instr[1];
					input[i].valid = false;
				}
				else
				{
					if (fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) == EOF) {
						//printf("End of File Reached!\n");
						pl.eof = true;
						input = new instr[1];
						input[i].valid = false;
					}
					//If file had instruction load it into the input
					else {
						//printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
						//printf("Read in line: %d \n", traceLine);
						input[i].pc = pc;
						input[i].op_type = op_type;
						input[i].rd = dest;
						input[i].rs1 = src1;
						input[i].rs2 = src2;
						input[i].valid = true;
						input[i].traceLine = traceLine;
						traceLine++;
					}
				}
			}
		}


		//printf("  retire...\n");
		pl.retire();
		//printf("  writeback...\n");
		pl.writeback();
		//printf("  execute...\n");
		pl.execute();
		//printf("  issue...\n");
		pl.issue();
		//printf("  dispatch...\n");
		pl.dispatch();
		//printf("  regRead...\n");
		pl.regRead();
		//printf("  rename...\n");
		pl.rename();
		//printf("  decode...\n");
		pl.decode();
		//printf("  fetch...\n");
		pl.fetch(input);
		pl.cycle++;
	}

	printf("# === Simulator Command =========\n");
	printf("# ./sim %d %d %d %s\n", params.rob_size, params.iq_size, params.width, trace_file);
	printf("# === Processor Configuration ===\n");
	printf("# ROB_SIZE = %d\n", params.rob_size);
	printf("# IQ_SIZE  = %d\n", params.iq_size);
	printf("# WIDTH    = %d\n", params.width);
	printf("# === Simulation Results ========\n");
	printf("# Dynamic Instruction Count    = %d\n",traceLine);
	printf("# Cycles                       = %d\n",pl.cycle);
	printf("# Instructions Per Cycle (IPC) = %0.2f\n", 1.0*(traceLine+1)/pl.cycle);
	return 0;
}
