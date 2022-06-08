#ifndef LEARNED_INDEX_H
#define LEARNED_INDEX_H

#include <trident/index.h>
#include <trident/tree/coordinates.h>
#include <trident/tree/root.h>
#include <trident/utils/propertymap.h>

#include <alex/alex.h>

#include <string>
#include <istream>
#include <ostream>

class LearnedIndex//: public Index
{
public:
    LearnedIndex(Root& root, bool readOnly = false);
    LearnedIndex(const LearnedIndex& other) = default;
    LearnedIndex(LearnedIndex&& other) = default;
    void put(const nTerm& key, TermCoordinates& value);// override;
    void put(nTerm&& key, TermCoordinates& value);// override;
    bool get(const nTerm& key, TermCoordinates& value);// override;
    bool get(nTerm&& key, TermCoordinates& value);// override;

    // Operator equivalents of get()
    // Return cleared TermCoordinates
    TermCoordinates operator[](const nTerm& key);
    TermCoordinates operator[](nTerm&& key);

    // Iterator
    alex::Alex<nTerm, TermCoordinates>::Iterator begin();
    alex::Alex<nTerm, TermCoordinates>::Iterator end();

private:
    alex::Alex<nTerm, TermCoordinates> index;
    bool readOnly;
};

#endif