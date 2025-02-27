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

#pragma once

#include "Status.h"

#include <memory>
#include <string>
#include <vector>

/** \brief Milvus SDK namespace
 */
namespace milvus {

/**
 * @brief Index Type
 */
enum class IndexType {
    invalid = 0,
    cpu_idmap,
    gpu_ivfflat,
    gpu_ivfsq8,
    mix_nsg,
    ivfsq8h,
};

enum class MetricType {
    L2 = 1,
    IP = 2,
};

/**
 * @brief Connect API parameter
 */
struct ConnectParam {
    std::string ip_address;  ///< Server IP address
    std::string port;        ///< Server PORT
};

/**
 * @brief Table Schema
 */
struct TableSchema {
    std::string table_name;                   ///< Table name
    int64_t dimension = 0;                    ///< Vector dimension, must be a positive value
    int64_t index_file_size = 0;              ///< Index file size, must be a positive value
    MetricType metric_type = MetricType::L2;  ///< Index metric type
};

/**
 * @brief Range information
 * for DATE partition, the format is like: 'year-month-day'
 */
struct Range {
    std::string start_value;  ///< Range start
    std::string end_value;    ///< Range stop
};

/**
 * @brief Record inserted
 */
struct RowRecord {
    std::vector<float> data;  ///< Vector raw data
};

/**
 * @brief TopK query result
 */
struct TopKQueryResult {
    int64_t row_num;
    std::vector<int64_t> ids;
    std::vector<float> distances;
};

/**
 * @brief index parameters
 */
struct IndexParam {
    std::string table_name;
    IndexType index_type;
    int32_t nlist;
};

/**
 * @brief SDK main class
 */
class Connection {
 public:
    /**
     * @brief CreateConnection
     *
     * Create a connection instance and return it's shared pointer
     *
     * @return Connection instance pointer
     */

    static std::shared_ptr<Connection>
    Create();

    /**
     * @brief DestroyConnection
     *
     * Destroy the connection instance
     *
     * @param connection, the shared pointer to the instance to be destroyed
     *
     * @return if destroy is successful
     */

    static Status
    Destroy(std::shared_ptr<Connection>& connection_ptr);

    /**
     * @brief Connect
     *
     * Connect function should be called before any operations
     * Server will be connected after Connect return OK
     *
     * @param param, use to provide server information
     *
     * @return Indicate if connect is successful
     */

    virtual Status
    Connect(const ConnectParam& param) = 0;

    /**
     * @brief Connect
     *
     * Connect function should be called before any operations
     * Server will be connected after Connect return OK
     *
     * @param uri, use to provide server information, example: milvus://ipaddress:port
     *
     * @return Indicate if connect is successful
     */
    virtual Status
    Connect(const std::string& uri) = 0;

    /**
     * @brief connected
     *
     * Connection status.
     *
     * @return Indicate if connection status
     */
    virtual Status
    Connected() const = 0;

    /**
     * @brief Disconnect
     *
     * Server will be disconnected after Disconnect return OK
     *
     * @return Indicate if disconnect is successful
     */
    virtual Status
    Disconnect() = 0;

    /**
     * @brief Create table method
     *
     * This method is used to create table
     *
     * @param param, use to provide table information to be created.
     *
     * @return Indicate if table is created successfully
     */
    virtual Status
    CreateTable(const TableSchema& param) = 0;

    /**
     * @brief Test table existence method
     *
     * This method is used to create table
     *
     * @param table_name, table name is going to be tested.
     *
     * @return Indicate if table is cexist
     */
    virtual bool
    HasTable(const std::string& table_name) = 0;

    /**
     * @brief Delete table method
     *
     * This method is used to delete table.
     *
     * @param table_name, table name is going to be deleted.
     *
     * @return Indicate if table is delete successfully.
     */
    virtual Status
    DropTable(const std::string& table_name) = 0;

    /**
     * @brief Create index method
     *
     * This method is used to create index for whole table
     *
     * @param IndexParam
     *  table_name, table name is going to be create index.
     *  index type,
     *  nlist,
     *  index file size
     *
     * @return Indicate if build index successfully.
     */
    virtual Status
    CreateIndex(const IndexParam& index_param) = 0;

    /**
     * @brief Add vector to table
     *
     * This method is used to add vector array to table.
     *
     * @param table_name, table_name is inserted.
     * @param record_array, vector array is inserted.
     * @param id_array, after inserted every vector is given a id.
     *
     * @return Indicate if vector array are inserted successfully
     */
    virtual Status
    Insert(const std::string& table_name, const std::vector<RowRecord>& record_array,
           std::vector<int64_t>& id_array) = 0;

    /**
     * @brief Search vector
     *
     * This method is used to query vector in table.
     *
     * @param table_name, table_name is queried.
     * @param query_record_array, all vector are going to be queried.
     * @param query_range_array, time ranges, if not specified, will search in whole table
     * @param topk, how many similarity vectors will be searched.
     * @param topk_query_result_array, result array.
     *
     * @return Indicate if query is successful.
     */
    virtual Status
    Search(const std::string& table_name, const std::vector<RowRecord>& query_record_array,
           const std::vector<Range>& query_range_array, int64_t topk, int64_t nprobe,
           TopKQueryResult& topk_query_result) = 0;

    /**
     * @brief Show table description
     *
     * This method is used to show table information.
     *
     * @param table_name, which table is show.
     * @param table_schema, table_schema is given when operation is successful.
     *
     * @return Indicate if this operation is successful.
     */
    virtual Status
    DescribeTable(const std::string& table_name, TableSchema& table_schema) = 0;

    /**
     * @brief Get table row count
     *
     * This method is used to get table row count.
     *
     * @param table_name, table's name.
     * @param row_count, table total row count.
     *
     * @return Indicate if this operation is successful.
     */
    virtual Status
    CountTable(const std::string& table_name, int64_t& row_count) = 0;

    /**
     * @brief Show all tables in database
     *
     * This method is used to list all tables.
     *
     * @param table_array, all tables are push into the array.
     *
     * @return Indicate if this operation is successful.
     */
    virtual Status
    ShowTables(std::vector<std::string>& table_array) = 0;

    /**
     * @brief Give the client version
     *
     * This method is used to give the client version.
     *
     * @return Client version.
     */
    virtual std::string
    ClientVersion() const = 0;

    /**
     * @brief Give the server version
     *
     * This method is used to give the server version.
     *
     * @return Server version.
     */
    virtual std::string
    ServerVersion() const = 0;

    /**
     * @brief Give the server status
     *
     * This method is used to give the server status.
     *
     * @return Server status.
     */
    virtual std::string
    ServerStatus() const = 0;

    virtual std::string
    DumpTaskTables() const = 0;

    /**
     * @brief delete tables by range
     *
     * This method is used to delete tables by range.
     *
     * @param Range, table range to delete.
     * @param table_name
     *
     * @return Indicate if this operation is successful.
     */
    virtual Status
    DeleteByRange(Range& range, const std::string& table_name) = 0;

    /**
     * @brief preload table
     *
     * This method is used to preload table
     *
     * @param table_name
     *
     * @return Indicate if this operation is successful.
     */
    virtual Status
    PreloadTable(const std::string& table_name) const = 0;

    /**
     * @brief describe index
     *
     * This method is used to describe index
     *
     * @param table_name
     *
     * @return index informations and indicate if this operation is successful.
     */
    virtual Status
    DescribeIndex(const std::string& table_name, IndexParam& index_param) const = 0;

    /**
     * @brief drop index
     *
     * This method is used to drop index
     *
     * @param table_name
     *
     * @return Indicate if this operation is successful.
     */
    virtual Status
    DropIndex(const std::string& table_name) const = 0;
};

}  // namespace milvus
