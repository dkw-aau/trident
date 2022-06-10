#include <trident/learned_index/learnedindex.h>

#include <kognac/utils.h>

#include <utility>
#include <functional>
#include <memory>

LearnedIndex::LearnedIndex(Root& root, bool readOnly)
{
    std::unique_ptr<TreeItr> itr(root.itr());

    while (itr->hasNext())
    {
        TermCoordinates coord;
        nTerm key = itr->next(&coord);
        put(std::move(key), coord);
    }

    this->readOnly = readOnly;
}

void LearnedIndex::put(const nTerm& key, TermCoordinates& value)
{
    this->index[key] = value;
}

void LearnedIndex::put(nTerm&& key, TermCoordinates& value)
{
    this->index[std::move(key)] = value;
}

bool LearnedIndex::get(const nTerm& key, TermCoordinates& value)
{
    if (this->index.find(key) == this->index.end())
        return false;

    value = this->index[key];
    return true;
}

bool LearnedIndex::get(nTerm&& key, TermCoordinates& value)
{
    nTerm cKey = std::move(key);

    if (this->index.find(cKey) == this->index.end())
        return false;

    value = this->index[cKey];
    return true;
}

TermCoordinates LearnedIndex::operator[](const nTerm& key)
{
    TermCoordinates coordinates;

    if (!get(key, coordinates))
        coordinates.clear();

    return coordinates;
}

TermCoordinates LearnedIndex::operator[](nTerm&& key)
{
    TermCoordinates coordinates;

    if (!get(std::move(key), coordinates))
        coordinates.clear();

    return coordinates;
}

std::unordered_map<nTerm, TermCoordinates>::iterator LearnedIndex::begin()
{
    return this->index.begin();
}

std::unordered_map<nTerm, TermCoordinates>::iterator  LearnedIndex::end()
{
    return this->index.end();
}
