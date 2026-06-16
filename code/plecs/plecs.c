#include "DllHeader.h"
#include "plecs.h"
#include "stdint.h"
#include "section.h"
#include "math.h"

struct SimulationState *plecs_astate;
uint32_t plecs_time_1ms = 0;

float plecs_get_input(PLECS_INPUT_E num)
{
    if (num < PLECS_INPUT_MAX)
    {
        return (float)plecs_astate->inputs[num];
    }
}

void plecs_set_output(PLECS_OUTPUT_E num, float val)
{
    if (num < PLECS_OUTPUT_MAX)
    {
        plecs_astate->outputs[num] = val;
    }
}

DLLEXPORT void plecsSetSizes(struct SimulationSizes *aSizes)
{
    aSizes->numInputs = PLECS_INPUT_NUM;
    aSizes->numOutputs = PLECS_OUTPUT_NUM;
    aSizes->numStates = 0;
    aSizes->numParameters = 0;
}

DLLEXPORT void plecsStart(struct SimulationState *aState)
{
    plecs_astate = aState;
    section_init();
}

DLLEXPORT void plecsOutput(struct SimulationState *aState)
{
    plecs_astate = aState;
    static float time = 0.0f;
    static float time_last = 0.0f;
    time = plecs_get_input(PLECS_INPUT_SIM_TIME);
    if ((time - time_last) > 0.001f)
    {
        plecs_time_1ms += (uint32_t)((time - time_last) * 1000.0f);
        run_task();
        time_last = time;
    }
    section_interrupt();
}
