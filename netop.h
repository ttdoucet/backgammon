/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"

class netop
{
public:
    virtual void forward() = 0;
    virtual void dloss_dx() = 0;
    virtual void update_w() = 0;
    virtual ~netop() = default;
};

