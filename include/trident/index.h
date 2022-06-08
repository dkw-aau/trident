#ifndef INDEX_H
#define INDEX_H

#include <trident/tree/coordinates.h>

class Index
{
public:
    virtual ~Index() {}
    virtual void put(const nTerm& key, TermCoordinates& value) = 0;
    virtual void put(nTerm&& key, TermCoordinates& value) = 0;
    virtual bool get(const nTerm& key, TermCoordinates& value) = 0;
    virtual bool get(nTerm&& key, TermCoordinates& value) = 0;
};

#endif