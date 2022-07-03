#include <trident/learned_index/learnedindex.h>

#include <kognac/utils.h>

#include <utility>
#include <memory>
#include <chrono>
#include <fstream>

LearnedIndex::LearnedIndex(const std::string& file)
    : readOnly(false), path(path)
{
    auto start = std::chrono::steady_clock::now();

    if (Utils::exists(path))
    {
        load();
        std::chrono::duration<double, std::milli> d = std::chrono::steady_clock::now() - start;
        LOG(INFOL) << "Loaded learned index in " << d.count() << "ms";
    }

    else
        LOG(INFOL) << "No prior loaded learned index";
}

LearnedIndex::LearnedIndex(const std::string& file, Root& root, bool readOnly)
    : LearnedIndex(file)
{
    auto start = std::chrono::steady_clock::now();
    std::unique_ptr<TreeItr> itr(root.itr());

    while (itr->hasNext())
    {
        TermCoordinates coord;
        nTerm key = itr->next(&coord);
        put(std::move(key), coord);
    }

    std::chrono::duration<double, std::milli> d = std::chrono::steady_clock::now() - start;
    LOG(INFOL) << "Learned index loaded in from B+ tree in " << d.count() << "ms";
    this->readOnly = readOnly;
    persist();
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

alex::Alex<nTerm, TermCoordinates>::Iterator LearnedIndex::end()
{
    return this->index.end();
}

void LearnedIndex::persist() const
{
    std::ofstream stream(this->path.c_str(), std::ios::binary);
    stream.write((char*) &this->index, sizeof(this->index));
}

void LearnedIndex::load()
{
    this->index.clear();

    std::ifstream stream(this->path.c_str(), std::ios::binary);
    stream.read((char*) &this->index, sizeof(this->index));
}