#ifndef LEARNED_INDEX_H
#define LEARNED_INDEX_H

#include <trident/index.h>
#include <trident/tree/coordinates.h>
#include <trident/tree/root.h>
#include <trident/utils/propertymap.h>

#include <string>
#include <istream>
#include <ostream>
#include <unordered_map>

class LearnedIndex: public Index
{
public:
    LearnedIndex(Root& root, bool readOnly = false);
    LearnedIndex(const LearnedIndex& other) = default;
    LearnedIndex(LearnedIndex&& other) = default;
    void put(const nTerm& key, TermCoordinates& value) override;
    void put(nTerm&& key, TermCoordinates& value) override;
    bool get(const nTerm& key, TermCoordinates& value) override;
    bool get(nTerm&& key, TermCoordinates& value) override;

    // Operator equivalents of get()
    // Return cleared TermCoordinates
    TermCoordinates operator[](const nTerm& key);
    TermCoordinates operator[](nTerm&& key);

    // Iterator
    std::unordered_map<nTerm, TermCoordinates>::iterator begin();
    std::unordered_map<nTerm, TermCoordinates>::iterator end();

private:
    std::unordered_map<nTerm, TermCoordinates> index;
    bool readOnly;
};

#endif