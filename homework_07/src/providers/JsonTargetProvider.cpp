#include "providers/JsonTargetProvider.h"
#include "Types.h"

int JsonTargetProvider::getTargetCount() const
{
    // TODO: implement
    return 0;
}

Target& JsonTargetProvider::getTarget(int idx)
{
    // TODO: implement
    static Target dummy{};
    return dummy;
}