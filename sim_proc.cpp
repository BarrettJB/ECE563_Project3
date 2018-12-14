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
	printf("starting main...\n");
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    unsigned long int pc; // Variable holds the pc read from input file

    /*
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    */
    //FP = fopen(trace_file, "r");
    FP = fopen("test_trace.txt", "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    //Create the pipeline
    //instr* input = new instr[params.width];
    //Pipeline pl(params.width, params.iq_size, params.rob_size);
    printf("creating objects...\n");
    instr* input = new instr[1];
    Pipeline pl(1, 4, 4);
    int cycles = 0;
    while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
    {
        cycles++;
        printf("Cycle %d\n",cycles);
        printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
        /*************************************
            Add calls to OOO simulator here
        **************************************/
        printf("setting input values \n");
        input[0].pc = pc;
        input[0].op_type = op_type;
        input[0].rd = dest;
        input[0].rs1 = src1;
        input[0].rs2 = src2;
        input[0].valid = true;

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
    }

    printf("End of File Reached!\n");
    pl.eof = true;

    while(!pl.finished){
    	cycles++;
    	printf("Cycle %d\n",cycles);
    	input = new instr[1];
    	input[0].valid = false;
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
    }
    return 0;
}
