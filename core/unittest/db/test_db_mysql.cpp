// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "db/utils.h"
#include "db/DB.h"
#include "db/DBImpl.h"
#include "db/Constants.h"
#include "db/meta/MetaConsts.h"

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include <thread>
#include <random>

namespace {

static const char *TABLE_NAME = "test_group";
static constexpr int64_t TABLE_DIM = 256;
static constexpr int64_t VECTOR_COUNT = 25000;
static constexpr int64_t INSERT_LOOP = 1000;

milvus::engine::meta::TableSchema
BuildTableSchema() {
    milvus::engine::meta::TableSchema table_info;
    table_info.dimension_ = TABLE_DIM;
    table_info.table_id_ = TABLE_NAME;
    table_info.engine_type_ = (int) milvus::engine::EngineType::FAISS_IDMAP;
    return table_info;
}

void
BuildVectors(int64_t n, std::vector<float> &vectors) {
    vectors.clear();
    vectors.resize(n * TABLE_DIM);
    float *data = vectors.data();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < TABLE_DIM; j++) data[TABLE_DIM * i + j] = drand48();
        data[TABLE_DIM * i] += i / 2000.;
    }
}

} // namespace

TEST_F(MySqlDBTest, DB_TEST) {
    milvus::engine::meta::TableSchema table_info = BuildTableSchema();
    auto stat = db_->CreateTable(table_info);

    milvus::engine::meta::TableSchema table_info_get;
    table_info_get.table_id_ = TABLE_NAME;
    stat = db_->DescribeTable(table_info_get);
    ASSERT_TRUE(stat.ok());
    ASSERT_EQ(table_info_get.dimension_, TABLE_DIM);

    milvus::engine::IDNumbers vector_ids;
    milvus::engine::IDNumbers target_ids;

    int64_t nb = 50;
    std::vector<float> xb;
    BuildVectors(nb, xb);

    int64_t qb = 5;
    std::vector<float> qxb;
    BuildVectors(qb, qxb);

    db_->InsertVectors(TABLE_NAME, qb, qxb.data(), target_ids);
    ASSERT_EQ(target_ids.size(), qb);

    std::thread search([&]() {
        milvus::engine::ResultIds result_ids;
        milvus::engine::ResultDistances result_distances;
        int k = 10;
        std::this_thread::sleep_for(std::chrono::seconds(5));

        INIT_TIMER;
        std::stringstream ss;
        uint64_t count = 0;
        uint64_t prev_count = 0;

        for (auto j = 0; j < 10; ++j) {
            ss.str("");
            db_->Size(count);
            prev_count = count;

            START_TIMER;
            stat = db_->Query(TABLE_NAME, k, qb, 10, qxb.data(), result_ids, result_distances);
            ss << "Search " << j << " With Size " << count / milvus::engine::M << " M";
            STOP_TIMER(ss.str());

            ASSERT_TRUE(stat.ok());
            for (auto i = 0; i < qb; ++i) {
//                std::cout << results[k][0].first << " " << target_ids[k] << std::endl;
//                ASSERT_EQ(results[k][0].first, target_ids[k]);
                bool exists = false;
                for (auto t = 0; t < k; t++) {
                    if (result_ids[i * k + t] == target_ids[i]) {
                        exists = true;
                    }
                }
                ASSERT_TRUE(exists);
                ss.str("");
                ss << "Result [" << i << "]:";
                for (auto t = 0; t < k; t++) {
                    ss << result_ids[i * k + t] << " ";
                }
                /* LOG(DEBUG) << ss.str(); */
            }
            ASSERT_TRUE(count >= prev_count);
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

        std::cout << "All search done!" << std::endl;
    });

    int loop = INSERT_LOOP;

    for (auto i = 0; i < loop; ++i) {
//        if (i==10) {
//            db_->InsertVectors(TABLE_NAME, qb, qxb.data(), target_ids);
//            ASSERT_EQ(target_ids.size(), qb);
//        } else {
//            db_->InsertVectors(TABLE_NAME, nb, xb.data(), vector_ids);
//        }
        db_->InsertVectors(TABLE_NAME, nb, xb.data(), vector_ids);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    search.join();
}

TEST_F(MySqlDBTest, SEARCH_TEST) {
    milvus::engine::meta::TableSchema table_info = BuildTableSchema();
    auto stat = db_->CreateTable(table_info);

    milvus::engine::meta::TableSchema table_info_get;
    table_info_get.table_id_ = TABLE_NAME;
    stat = db_->DescribeTable(table_info_get);
    ASSERT_TRUE(stat.ok());
    ASSERT_EQ(table_info_get.dimension_, TABLE_DIM);

    // prepare raw data
    size_t nb = VECTOR_COUNT;
    size_t nq = 10;
    size_t k = 5;
    std::vector<float> xb(nb * TABLE_DIM);
    std::vector<float> xq(nq * TABLE_DIM);
    std::vector<int64_t> ids(nb);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_xt(-1.0, 1.0);
    for (size_t i = 0; i < nb * TABLE_DIM; i++) {
        xb[i] = dis_xt(gen);
        if (i < nb) {
            ids[i] = i;
        }
    }
    for (size_t i = 0; i < nq * TABLE_DIM; i++) {
        xq[i] = dis_xt(gen);
    }

    // result data
    //std::vector<long> nns_gt(k*nq);
    std::vector<int64_t> nns(k * nq);  // nns = nearst neg search
    //std::vector<float> dis_gt(k*nq);
    std::vector<float> dis(k * nq);

    // insert data
    const int batch_size = 100;
    for (int j = 0; j < nb / batch_size; ++j) {
        stat = db_->InsertVectors(TABLE_NAME, batch_size, xb.data() + batch_size * j * TABLE_DIM, ids);
        if (j == 200) { sleep(1); }
        ASSERT_TRUE(stat.ok());
    }

    sleep(2); // wait until build index finish

    milvus::engine::ResultIds result_ids;
    milvus::engine::ResultDistances result_distances;
    stat = db_->Query(TABLE_NAME, k, nq, 10, xq.data(), result_ids, result_distances);
    ASSERT_TRUE(stat.ok());
}

TEST_F(MySqlDBTest, ARHIVE_DISK_CHECK) {
    milvus::engine::meta::TableSchema table_info = BuildTableSchema();
    auto stat = db_->CreateTable(table_info);

    std::vector<milvus::engine::meta::TableSchema> table_schema_array;
    stat = db_->AllTables(table_schema_array);
    ASSERT_TRUE(stat.ok());
    bool bfound = false;
    for (auto &schema : table_schema_array) {
        if (schema.table_id_ == TABLE_NAME) {
            bfound = true;
            break;
        }
    }
    ASSERT_TRUE(bfound);

    milvus::engine::meta::TableSchema table_info_get;
    table_info_get.table_id_ = TABLE_NAME;
    stat = db_->DescribeTable(table_info_get);
    ASSERT_TRUE(stat.ok());
    ASSERT_EQ(table_info_get.dimension_, TABLE_DIM);

    milvus::engine::IDNumbers vector_ids;
    milvus::engine::IDNumbers target_ids;

    uint64_t size;
    db_->Size(size);

    int64_t nb = 10;
    std::vector<float> xb;
    BuildVectors(nb, xb);

    int loop = INSERT_LOOP;
    for (auto i = 0; i < loop; ++i) {
        db_->InsertVectors(TABLE_NAME, nb, xb.data(), vector_ids);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    db_->Size(size);
    LOG(DEBUG) << "size=" << size;
    ASSERT_LE(size, 1 * milvus::engine::G);
}

TEST_F(MySqlDBTest, DELETE_TEST) {
    milvus::engine::meta::TableSchema table_info = BuildTableSchema();
    auto stat = db_->CreateTable(table_info);
//    std::cout << stat.ToString() << std::endl;

    milvus::engine::meta::TableSchema table_info_get;
    table_info_get.table_id_ = TABLE_NAME;
    stat = db_->DescribeTable(table_info_get);
    ASSERT_TRUE(stat.ok());

    bool has_table = false;
    db_->HasTable(TABLE_NAME, has_table);
    ASSERT_TRUE(has_table);

    milvus::engine::IDNumbers vector_ids;

    uint64_t size;
    db_->Size(size);

    int64_t nb = INSERT_LOOP;
    std::vector<float> xb;
    BuildVectors(nb, xb);

    int loop = 20;
    for (auto i = 0; i < loop; ++i) {
        db_->InsertVectors(TABLE_NAME, nb, xb.data(), vector_ids);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

//    std::vector<engine::meta::DateT> dates;
//    stat = db_->DeleteTable(TABLE_NAME, dates);
////    std::cout << "5 sec start" << std::endl;
//    std::this_thread::sleep_for(std::chrono::seconds(5));
////    std::cout << "5 sec finish" << std::endl;
//    ASSERT_TRUE(stat.ok());
//
//    db_->HasTable(TABLE_NAME, has_table);
//    ASSERT_FALSE(has_table);
}

