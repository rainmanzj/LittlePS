#ifndef LITTLEPHOTOSHOP_OBSERVER_H
#define LITTLEPHOTOSHOP_OBSERVER_H


#include "Params.h"

class Observer {
public:
    virtual void update(Params params) = 0;
};

#endif //LITTLEPHOTOSHOP_OBSERVER_H
