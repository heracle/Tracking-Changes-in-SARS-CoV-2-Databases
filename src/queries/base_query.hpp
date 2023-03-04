#pragma once

#include <string>
#include "../common/logger.hpp"

#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/static_types/static_base.hpp"
#include "../ds/database.hpp"

namespace query_ns {

enum TreeDirectionToGo {
    NoSubtree = 0,
    LeftChild = 1,
    RightChild = 2,
};

class DataQueryObj {
  private:
    enum Type {kLEAF_UNSET, kLEAF_INT, kLEAF_STR, kNODE_MAP, kNODE_ARR};
    Type type = kLEAF_UNSET;
    tsl::hopscotch_map<std::string, DataQueryObj*> data_map;
    std::vector<std::string> ordered_datamap_keys;
    std::vector<DataQueryObj*> data_arr;
    std::string val_str;
    int64_t val_int;
    static const uint64_t spacing = 2;

    void AddIndentToStr(uint64_t level, std::string &s);
    void AppendDataToStr(uint64_t level, std::string &s);
  public:
    DataQueryObj& operator()(const std::string &label);
    DataQueryObj& operator[](const uint64_t &label);
    void SetValStr(const std::string &s);
    std::string GetValStr();
    void SetValInt(const int64_t &s);
    int64_t GetValInt();
    uint64_t GetNumChildren();
    DataQueryObj();
    void ClearData();
    std::string GetJsonStr(uint64_t level = 0);
};

class BaseQuery {
  public:
    bool deletions_mode = false;
    uint64_t snapshot_idx = 0;
    std::string snapshot_current_name;
    DataQueryObj saved_data;
    // init does some initialisation for the specific query and returns the name of the treap to run.
    virtual std::string get_treap_name() = 0;
    virtual void reset() = 0;
    virtual TreeDirectionToGo first_enter_into_node(const std::string &, treap_types::Tnode *, const treap_types::BaseSortedTreap *, const ds::DB *) = 0;
    virtual TreeDirectionToGo second_enter_into_node(const std::string &, treap_types::Tnode *, const treap_types::BaseSortedTreap *, const ds::DB *) = 0;
    virtual void post_process(){};
    virtual void set_deletion_mode(const bool is_deletion_mode) = 0;
    virtual ~BaseQuery(){};
};

} // namespace query_ns