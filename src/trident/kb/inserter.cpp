/*
 * Copyright 2017 Jacopo Urbani
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
**/


#include <trident/kb/inserter.h>
#include <trident/binarytables/storagestrat.h>
#include <trident/binarytables/tableshandler.h>
#include <trident/binarytables/binarytableinserter.h>

//means: newrow layout with 8 bytes + 8 bytes
char Inserter::STRATEGY_FOR_POS =
StorageStrat::getStrategy(NEWROW_ITR, WO_DIFFERENCE, 3, 4, true);

bool Inserter::insert(const int permutation,
        const int64_t t1,
        const int64_t t2,
        const int64_t t3,
        const int64_t count,
        TripleWriter *posArray,
        LearnedIndexInserter* learnedIndexInserter,
        const bool aggregated,
        const bool canSkipTables) {

    bool ret = false;
    if (t1 != currentT1[permutation]) {
        if (t1 < currentT1[permutation]) {
            LOG(DEBUGL) << "t1=" << t1 << " currentT1[perm]=" << currentT1[permutation];
        }
        ntables[permutation]++;
        nFirstElsNTables[permutation]++;
        if (currentT1[permutation] != -1) {
            writeCurrentEntryIntoLearnedIndex(permutation, posArray,
                    learnedIndexInserter, aggregated, canSkipTables);
        }
        currentT1[permutation] = t1;
        currentT2[permutation] = t2;
        nElements[permutation] = 0;
        posElements[permutation] = 0;

        if (posArray != NULL) {
            lastFirstTerm[permutation] = -1;
            countLastFirstTerm[permutation] = 0;
            coordinatesLastFirstTerm[permutation] = 0;
        }
    } else if (t2 != currentT2[permutation]) {
        nFirstElsNTables[permutation]++;
        currentT2[permutation] = t2;
    }

    int64_t n = nElements[permutation]++;
    if (aggregated) {
        posElements[permutation] += count;
    }
    if (n < THRESHOLD_KEEP_MEMORY) {
        // Copy the values in an internal buffer
        values1[permutation][(int) n] = t2;
        values2[permutation][(int) n] = t3;
    } else {
        if (n == THRESHOLD_KEEP_MEMORY) {
            storeInmemoryValuesIntoFiles(permutation, values1[permutation],
                    values2[permutation], (int) n, posArray,
                    aggregated, canSkipTables);
        }
        if (skipTable[permutation]) {
            throw 10; //should never happen
        }

        if (posArray != NULL) {
            if (t2 != lastFirstTerm[permutation]) { // store element into the list
                if (lastFirstTerm[permutation] != -1) {
                    posArray->write(lastFirstTerm[permutation], currentT1[permutation]
                            , coordinatesLastFirstTerm[permutation],
                            countLastFirstTerm[permutation]);
                }
                coordinatesLastFirstTerm[permutation] = getCoordinatesForPOS(permutation);
                lastFirstTerm[permutation] = t2;
                countLastFirstTerm[permutation] = 0;
            }
            posArray->write(t2, t1, t3 | POSAGGRBYTE, 1);
            countLastFirstTerm[permutation]++;
        }

        if (aggregated) {
            if (onlyReferences[permutation]) {
                if ((t3 & POSAGGRBYTE) == 0) {
                    files[permutation]->append(t2, t3);
                }
            } else {
                if (t3 & POSAGGRBYTE) {
                    files[permutation]->append(t2, t3 & ~POSAGGRBYTE);
                }
            }
        } else {
            files[permutation]->append(t2, t3);
        }

    }
    return ret;
}

std::string Inserter::getPathPermutationStorage(const int perm) {
    return files[perm]->getPath();
}

int64_t Inserter::getCoordinatesForPOS(const int p) {
    int64_t coordinates = ((int64_t) (strategies[p] & 0xFF) << 48) + ((int64_t) fileIdx[p] << 32)
        + startPositions[p];
    return coordinates;
}

void Inserter::insert(nTerm key, TermCoordinates *value) {
    this->learnedIndex->put(key, *value);
}

void Inserter::writeCurrentEntryIntoLearnedIndex(int permutation,
        TripleWriter *posArray, LearnedIndexInserter *learnedIndexInserter,
        const bool aggregated,
        const bool canSkipTables) {

    if (nElements[permutation] <= THRESHOLD_KEEP_MEMORY) {
        storeInmemoryValuesIntoFiles(permutation, values1[permutation],
                values2[permutation],
                (int) nElements[permutation],
                posArray, aggregated, canSkipTables);
    }

    if (!skipTable[permutation]) {
        if (posArray != NULL && lastFirstTerm[permutation] != -1) {
            posArray->write(lastFirstTerm[permutation], currentT1[permutation],
                    coordinatesLastFirstTerm[permutation], countLastFirstTerm[permutation]);
        }

        //Stop append
        files[permutation]->stopAppend();

        //Release PairHandler
        switch (currentPairHandler[permutation]->getType()) {
            case ROW_ITR:
                listFactory[permutation].release(
                        (RowTableInserter *) (currentPairHandler[permutation]));
                break;
            case CLUSTER_ITR:
                comprFactory[permutation].release(
                        (ClusterTableInserter *) (currentPairHandler[permutation]));
                break;
            case COLUMN_ITR:
                list2Factory[permutation].release(
                        (ColumnTableInserter *) (currentPairHandler[permutation]));
                break;
            case NEWCOLUMN_ITR:
                ncFactory[permutation].release((NewColumnTableInserter *) (currentPairHandler[permutation]));
                break;
            case NEWROW_ITR:
                nrFactory[permutation].release((NewRowTableInserter *) (currentPairHandler[permutation]));
                break;
            case NEWCLUSTER_ITR:
                ncluFactory[permutation].release((NewClusterTableInserter *) (currentPairHandler[permutation]));
                break;
        }

        int64_t nels;
        if (aggregated) {
            nels = posElements[permutation];
            nels /= 2; //I divide it by two because each table receives twice number of counts. One of each rows, and one for the aggregated ones.
            posElements[permutation] = 0;
        } else {
            nels = nElements[permutation];
        }

        learnedIndexInserter->addEntry(currentT1[permutation], nels,
                fileIdx[permutation],
                startPositions[permutation],
                strategies[permutation]);
    }
}

void Inserter::storeInmemoryValuesIntoFiles(int permutation, int64_t* v1, int64_t* v2,
        int n, TripleWriter *posArray, const bool aggregated,
        const bool canSkipTables) {
    //Can I skip the storage of the table?
    skipTable[permutation] = false;
    if (permutation == IDX_SOP || permutation == IDX_PSO ||
            permutation == IDX_OSP) {
        if (canSkipTables && n < thresholdSkipTable) {
            //The table is small enough to be skipped
            skipTable[permutation] = true;
            skippedTables[permutation]++;
            return;
        }
    }

    //Determine the storage strategy to use and register the coordinates
    char strat;
    if (useFixedStrategy) {
        strat = fixedStrategy;
        onlyReferences[permutation] = false;
    } else {
        //Should I store only the reference or the elements directly?
        onlyReferences[permutation] = aggregated && StorageStrat::determineAggregatedStrategy(v1, v2, n, nTerms, stats[permutation]);
        if (onlyReferences[permutation]) {
            strat = STRATEGY_FOR_POS;
        } else {
            strat = StorageStrat::determineStrategy(v1, v2, n, nTerms,
                    thresholdForColumnStorage,
                    useRowForLargeTables,
                    stats[permutation]);
        }
    }

    currentPairHandler[permutation] =
        storageStrategy[permutation].getBinaryTableInserter(strat);
    strategies[permutation] = strat;
    startPositions[permutation] = files[permutation]->startAppend(
            currentT1[permutation],
            strategies[permutation],
            currentPairHandler[permutation]);
    fileIdx[permutation] = files[permutation]->getLastCreatedFile();

    // Copy the values in-memory to disk
    for (int i = 0; i < n; ++i) {
        if (posArray != NULL) {
            if (v1[i] != lastFirstTerm[permutation]) {
                // store element into the list
                if (lastFirstTerm[permutation] != -1) {
                    posArray->write(lastFirstTerm[permutation], currentT1[permutation],
                            coordinatesLastFirstTerm[permutation],
                            countLastFirstTerm[permutation]);
                }
                lastFirstTerm[permutation] = v1[i];
                countLastFirstTerm[permutation] = 0;
                coordinatesLastFirstTerm[permutation] = getCoordinatesForPOS(permutation);
            }
            posArray->write(v1[i], currentT1[permutation], v2[i] | POSAGGRBYTE, 1);
            countLastFirstTerm[permutation]++;
        }
        if (aggregated) {
            if (onlyReferences[permutation]) {
                if ((v2[i] & POSAGGRBYTE) == 0) {
                    files[permutation]->append(v1[i], v2[i]);
                }
            } else {
                if (v2[i] & POSAGGRBYTE) {
                    files[permutation]->append(v1[i], v2[i] & ~POSAGGRBYTE);
                }
            }
        } else {
            files[permutation]->append(v1[i], v2[i]);
        }
    }
}

void Inserter::flush(int permutation, TripleWriter * posArray,
        LearnedIndexInserter* learnedIndexInserter, const bool aggregated,
        const bool canSkipTables) {
    if (currentT1[permutation] != -1) {
        writeCurrentEntryIntoLearnedIndex(permutation, posArray, learnedIndexInserter,
                aggregated, canSkipTables);
    }
    currentT1[permutation] = -1;
    this->learnedIndex->persist();
}

void Inserter::stopInserts(int permutation) {
    files[permutation]->stopInsert();
}
