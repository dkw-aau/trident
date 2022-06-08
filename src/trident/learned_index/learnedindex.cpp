#include <trident/learned_index/learnedindex.h>

#include <kognac/utils.h>

#include <utility>
#include <functional>
#include <memory>

LearnedIndex::LearnedIndex(Root& root, bool readOnly)
    : readOnly(readOnly)
{
    std::unique_ptr<TreeItr> itr(root.itr());

    while (itr->hasNext())
    {
        TermCoordinates coord;
        nTerm key = itr->next(&coord);
        put(std::move(key), coord);
    }
}

void LearnedIndex::put(const nTerm& key, TermCoordinates& value)
{
    if (this->readOnly)
    {
        LOG(ERRORL) << "Put is requested on read-only learned index";
        throw 10;
    }

    this->index.insert(key, value);
}

void LearnedIndex::put(nTerm&& key, TermCoordinates& value)
{
    if (this->readOnly)
    {
        LOG(ERRORL) << "Put is requested on read-only learned index";
        throw 10;
    }

    this->index.insert(std::move(key), value);
}

bool LearnedIndex::get(const nTerm& key, TermCoordinates& value)
{
    auto it = this->index.find(key);

    if (it != this->index.end())
    {
        value = (*it).second;
        return true;
    }

    return false;
}

bool LearnedIndex::get(nTerm&& key, TermCoordinates& value)
{
    auto it = this->index.find(std::move(key));

    if (it != this->index.end())
    {
        value = (*it).second;
        return true;
    }

    return false;
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

alex::Alex<nTerm, TermCoordinates>::Iterator LearnedIndex::begin()
{
    return this->index.begin();
}

alex::Alex<nTerm, TermCoordinates>::Iterator  LearnedIndex::end()
{
    return this->index.end();
}
