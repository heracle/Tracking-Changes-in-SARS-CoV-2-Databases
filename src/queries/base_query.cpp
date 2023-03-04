#include "base_query.hpp"

#include <algorithm>

namespace query_ns {

void DataQueryObj::AddIndentToStr(uint64_t level, std::string &s) {
    for (uint64_t i = 0; i < level * spacing; ++i) {
        s += " ";
    }
}

void DataQueryObj::AppendDataToStr(uint64_t level, std::string &s) {
    if (type == kLEAF_UNSET) {
        s += "\"\"";
        return;
    }
    else if (type == kLEAF_STR) {
        s += "\"" + val_str + "\"";
        return;
    }
    else if (type == kLEAF_INT) {
        s += std::to_string(val_int) + "";
        return;
    }
    else if (type == kNODE_MAP) {
        s += "{\n";
        std::sort(ordered_datamap_keys.begin(), ordered_datamap_keys.end());
        for (const std::string &key : ordered_datamap_keys) {
            AddIndentToStr(level + 1, s);
            s += "\"" + key + "\": ";
            data_map.at(key)->AppendDataToStr(level+1, s);
            if (key != ordered_datamap_keys.back()) {
                s += ",\n";
            }
        }
        s += "\n";
        AddIndentToStr(level, s);
        s += "}";
        return;
    }
    else if (type == kNODE_ARR) {
        s += "[\n";
        for (const auto &dqo : data_arr) {
            AddIndentToStr(level + 1, s);
            dqo->AppendDataToStr(level + 1, s);
            if (dqo != data_arr.back()) {
                s += ",\n";
            }
        }
        s += "\n";
        AddIndentToStr(level, s);
        s += "]";
        return;
    }
    
    Logger::error("This should never happen, maybe an unrecognized type?");
    exit(1);
}

DataQueryObj& DataQueryObj::operator()(const std::string &label) {
    if (type == kLEAF_UNSET) {
        type = kNODE_MAP;
    }
    if (type != kNODE_MAP) {
        Logger::error("Cannot run operator() on DataQuery object that is not kNODE_MAP.");
        exit(1);
    }
    auto it = data_map.find(label);
    if (it != data_map.end()) {
        return *(it->second);
    }
    data_map.emplace(label, new DataQueryObj());
    ordered_datamap_keys.push_back(label);
    return *(data_map.at(label));
}

DataQueryObj& DataQueryObj::operator[](const uint64_t &index) {
    if (type == kLEAF_UNSET) {
        type = kNODE_ARR;
    }
    if (type != kNODE_ARR) {
        Logger::error("Cannot run operator[] on DataQuery object that is not kNODE_ARR.");
        exit(1);
    }
    if (data_arr.size() < index) {
        Logger::error("Cannot allocate nonconsecutive values for data_arr vector.");
        exit(1);
    }
    if (data_arr.size() == index) {
        data_arr.emplace_back(new DataQueryObj());
    }
    return *(data_arr[index]);
}

void DataQueryObj::SetValStr(const std::string &s) {
    if (type == kLEAF_UNSET) {
        type = kLEAF_STR;
    }
    if (type != kLEAF_STR) {
        Logger::error("Cannot set value for DataQuery object that is not LEAF_STR.");
        exit(1);
    }
    val_str = s;
}

std::string DataQueryObj::GetValStr() {
    if (type == kLEAF_UNSET) {
        type = kLEAF_STR;
    }
    if (type != kLEAF_STR) {
        Logger::error("Cannot set value for DataQuery object that is not LEAF_STR.");
        exit(1);
    }
    return val_str;
}

void DataQueryObj::SetValInt(const int64_t &s) {
    if (type == kLEAF_UNSET) {
        type = kLEAF_INT;
    }
    if (type != kLEAF_INT) {
        Logger::error("Cannot set value for DataQuery object that is not LEAF_INT.");
        exit(1);
    }
    val_int = s;
}

int64_t DataQueryObj::GetValInt() {
    if (type == kLEAF_UNSET) {
        type = kLEAF_INT;
    }
    if (type != kLEAF_INT) {
        Logger::error("Cannot get value for DataQuery object that is not LEAF_INT.");
        exit(1);
    }
    return val_int;
}

uint64_t DataQueryObj::GetNumChildren() {
    return std::max(data_map.size(), data_arr.size());
}

DataQueryObj::DataQueryObj() {
    val_str = "";
    val_int = 0;
    type = kLEAF_UNSET;
}

void DataQueryObj::ClearData() {
    for (auto &it : data_map) {
        it.second->ClearData();
        delete it.second;
    }
    data_map.clear();
    ordered_datamap_keys.clear();
    for (auto &it : data_arr) {
        it->ClearData();
        delete it;
    }
    data_arr.clear();
}

std::string DataQueryObj::GetJsonStr(uint64_t level) {
    std::string result;
    AppendDataToStr(level, result);
    // New line at the end of the file;
    result += "\n";
    return result;
}

} // namespace query_ns
