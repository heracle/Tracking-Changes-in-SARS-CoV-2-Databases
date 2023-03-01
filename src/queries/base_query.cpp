#include "base_query.hpp"

#include <algorithm>

namespace query_ns {

void DataQueryObj::CloseBracket(uint64_t level, std::string &s) {
    for (uint64_t i = 0; i < level * spacing; ++i) {
        s += " ";
    }
    s += "},\n";
}

void DataQueryObj::AddDataLabelToStr(uint64_t level, const std::string &label, std::string &s) {
    for (uint64_t i = 0; i < level * spacing; ++i) {
        s += " ";
    }
    s += "\"" + label + "\": ";
}

void DataQueryObj::AppendDataToStr(uint64_t level, std::string &s) {
    if (type == kLEAF_UNSET) {
        s += "\"\",\n";
        return;
    }
    if (type == kLEAF_STR) {
        s += "\"" + val_str + "\",\n";
        return;
    }
    if (type == kLEAF_INT) {
        s += std::to_string(val_int) + ",\n";
        return;
    }
    s += "{\n";
    std::sort(ordered_data_keys.begin(), ordered_data_keys.end());
    for (const std::string &key : ordered_data_keys) {
        AddDataLabelToStr(level + 1, key, s);
        data.at(key)->AppendDataToStr(level+1, s);
    }
    CloseBracket(level, s);
}

DataQueryObj& DataQueryObj::operator()(const std::string &label) {
    if (type == kLEAF_UNSET) {
        type = kNON_LEAF;
    }
    if (type != kNON_LEAF) {
        Logger::error("Cannot run operator() on DataQuery object that is not NON_LEAF.");
        exit(1);
    }
    auto it = data.find(label);
    if (it != data.end()) {
        return *(it->second);
    }
    data.emplace(label, new DataQueryObj());
    ordered_data_keys.push_back(label);
    return *(data.at(label));
}

DataQueryObj& DataQueryObj::operator()(const uint64_t &label) {
    return this->operator()(std::to_string(label));
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
    return data.size();
}

DataQueryObj::DataQueryObj() {
    val_str = "";
    val_int = 0;
    type = kLEAF_UNSET;
}

void DataQueryObj::ClearData() {
    for (auto &it : data) {
        it.second->ClearData();
        delete it.second;
    }
    data.clear();
    ordered_data_keys.clear();
}

std::string DataQueryObj::GetJsonStr(uint64_t level) {
    std::string result;
    AppendDataToStr(level, result);
    return result;
}

} // namespace query_ns
