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


#ifndef _TERMITR_H_
#define _TERMITR_H_

#include <trident/binarytables/tableshandler.h>
#include <trident/iterators/pairitr.h>
#include <trident/tree/coordinates.h>
#include <trident/kb/consts.h>

class Root;

class TermItr: public PairItr {

    private:
        TableStorage *tables;
        int perm;
        uint64_t size;
        Root* tree;
        size_t nfiles;

        size_t currentfile;
        int64_t currentMark;
        int64_t cachedCount;
        const char *buffer;
        const char *endbuffer;

        size_t m_currentfile;
        int64_t m_currentMark;
        int64_t m_cachedCount;
        const char *m_buffer;
        const char *m_endbuffer;
        uint64_t m_key;

    public:
        void init(TableStorage *tables, uint64_t size, int perm, Root* tree);

        int getTypeItr() {
            return TERM_ITR;
        }

        short getCurrentFile() {
            return currentfile;
        }

        int64_t getCurrentMark() {
            return currentMark;
        }

        char getCurrentStrat();

        int64_t getValue1() {
            return 0;
        }

        int64_t getValue2() {
            return 0;
        }

        void ignoreSecondColumn() {
            throw 10; //not supported
        }

        int64_t getCount();

        bool hasNext();

        void next();

        void clear();

        uint64_t getCardinality();

        uint64_t estCardinality();

        void mark();

        void reset(const char i);

        void gotoKey(int64_t c);

        void moveto(const int64_t c1, const int64_t c2);
};

#endif
