#include "ppstepbuffer.h"

PPStepBuffer::PPStepBuffer()
{
}

PPStepBuffer::~PPStepBuffer()
{
}


unsigned long PPStepBuffer::numberOfSignals()
{
    int n1 = data.shape()[0];
    int n2 = data.shape()[1];
    return n1*n2;
}

unsigned long PPStepBuffer::numberOfDataPoints()
{
    int n1 = data.shape()[0];
    int n2 = data.shape()[1];
    unsigned long sum = 0;
    for(int m = 0; m < n1; m++)
        for(int c = 0; c < n2; c++)
            sum += data[m][c].data.size();
    return sum;
}
