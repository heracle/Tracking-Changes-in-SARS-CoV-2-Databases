#include "h5_helper.hpp"

#include "constants.hpp"
#include "logger.hpp"
#include <iostream>

namespace H5Helper {

H5::Attribute attr;
H5::IntType int64_datatype(H5::PredType::STD_I64LE);
H5::IntType uint64_datatype(H5::PredType::STD_U64LE);
H5::StrType str_datatype(H5::PredType::C_S1, H5T_VARIABLE);
H5::DataSpace simple_dataspace(H5S_SCALAR);

int64_t get_int64_attr_from(const H5::Group &h5, const std::string &attr_name) {
    int64_t value;
    attr = h5.openAttribute(attr_name);
    attr.read(int64_datatype, &value);
    return value;
}

uint64_t get_uint64_attr_from(const H5::Group &h5, const std::string &attr_name) {
    uint64_t value;
    attr = h5.openAttribute(attr_name);
    attr.read(uint64_datatype, &value);
    return value;
}

std::string get_string_attr_from(const H5::Group &h5, const std::string &attr_name) {
    std::string value;
    attr = h5.openAttribute(attr_name);
    attr.read(str_datatype, value);
    return value;
}

void set_int64_hdf5_attr(const int64_t value, H5::Group *h5, const std::string &attr_name) {
    if (h5->attrExists(attr_name)) {
        h5->removeAttr(attr_name);
    }
    
    attr = h5->createAttribute(attr_name, int64_datatype, simple_dataspace);
    attr.write(int64_datatype, &value);
}

void set_uint64_hdf5_attr(const uint64_t value, H5::Group *h5, const std::string &attr_name) {
    if (h5->attrExists(attr_name)) {
        h5->removeAttr(attr_name);
    }

    attr = h5->createAttribute(attr_name, uint64_datatype, simple_dataspace);
    attr.write(uint64_datatype, &value);
}

void set_string_hdf5_attr(const std::string value, H5::Group *h5, const std::string &attr_name) {
    if (h5->attrExists(attr_name)) {
        h5->removeAttr(attr_name);
    }
    
    attr = h5->createAttribute(attr_name, str_datatype, simple_dataspace);
    attr.write(str_datatype, value); 
}

// write_posstr_to_h5_dataset is an internal function for writing a "std::vector<std::string> v" to a h5 file.
// it receives a concatenated string for all the strings in the array (str_to_save) and an integer array (same size as v) 
// with all the indeces where the initial strings are starting in the concatenation.
void write_posstr_to_h5_dataset(const uint64_t elem_size, const uint64_t positions[], const std::string &str_to_save, H5::Group *h5, const std::string &dataset_name) {
    uint64_t RANK = 2;
    hsize_t dimsf[RANK];

    // write positions to hdf5

    dimsf[0] = 1;
    dimsf[1] = elem_size;

    hid_t dataspace = H5Screate_simple(RANK, dimsf, NULL);
    hid_t datatype = H5Tcopy(H5T_STD_U64LE);

    herr_t status = H5Tset_order(datatype, H5T_ORDER_LE);
    assert(status == 0);

    hid_t dataset = H5Dcreate(h5->getId(), (dataset_name + "_$_pos").c_str(), datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, positions);
    assert(status >= 0);

    // write str_to_save to hdf5

    dimsf[1] = str_to_save.size();
    dataspace = H5Screate_simple(RANK, dimsf, NULL);
    datatype = H5Tcopy(H5T_NATIVE_CHAR);

    dataset = H5Dcreate(h5->getId(), (dataset_name + "_$_str").c_str(), datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, str_to_save.c_str());
    assert(status >= 0);

    H5Tclose(datatype); 
    H5Dclose(dataset); 
    H5Sclose(dataspace);
}

// read_posstr_to_h5_dataset is an internal function for reading the searialization made by 'write_posstr_to_h5_dataset' function.
// It reads from a h5 file and returns a concatenated str and the indices where the small string are starting inside it.
// A furhter process will compose the final "std::vector<std::string> v".
void read_posstr_to_h5_dataset(const H5::Group &h5, const std::string &dataset_name, uint64_t *elem_size, uint64_t *&positions, std::string *loaded_str) {
    hsize_t dims_out[2];
    hsize_t max_dims[2];
    
    // read positions dataset
    hid_t dataset = H5Dopen(h5.getId(), (dataset_name + "_$_pos").c_str(), H5P_DEFAULT);
    hid_t datatype  = H5Dget_type(dataset);
    hid_t dataspace = H5Dget_space(dataset);    /* dataspace identifier */
    
    int rank = H5Sget_simple_extent_ndims(dataspace);
    assert (rank == 2);
    int status_n  = H5Sget_simple_extent_dims(dataspace, dims_out, max_dims);
    assert(status_n >= 0);

    *elem_size = dims_out[1];

    positions = new uint64_t[dims_out[0] * dims_out[1]];
    herr_t status = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, positions);
    assert(status >= 0);

    // read str dataset
    dataset = H5Dopen(h5.getId(), (dataset_name + "_$_str").c_str(), H5P_DEFAULT);
    datatype  = H5Dget_type(dataset);
    dataspace = H5Dget_space(dataset);

    rank = H5Sget_simple_extent_ndims(dataspace);
    assert (rank == 2);
    status_n  = H5Sget_simple_extent_dims(dataspace, dims_out, max_dims);
    assert(status_n >= 0);

    char *loaded_chars = new char[dims_out[0] * dims_out[1]];
    status = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, loaded_chars);
    assert(status >= 0);

    *loaded_str = std::string(loaded_chars);
    delete loaded_chars;

    H5Dclose(dataset);
    H5Tclose(datatype);
    H5Sclose(dataspace);
}

void create_extendable_h5_dataset(H5::Group &h5, const std::string &dataset_name) {
    uint64_t RANK = 1;
    hsize_t dimsf[RANK] = {1};
    hsize_t maxdims[RANK] = {H5S_UNLIMITED};

    /*
     set contiguous chuck size used for storing H5 into the memory;
    */
    hid_t cparms;
    hsize_t chunk_dims[RANK] = {common::H5_CHUNK_SIZE};

    cparms = H5Pcreate (H5P_DATASET_CREATE);
    herr_t status = H5Pset_chunk(cparms, RANK, chunk_dims);
    assert(status == 0);

    /* 
     create h5 space for storing the positions array.
    */
    hid_t dataspace = H5Screate_simple(RANK, dimsf, maxdims);
    hid_t dataset = H5Dcreate(h5.getId(), (dataset_name + "_$_pos").c_str(), H5T_NATIVE_LLONG, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
    assert (dataset != H5I_INVALID_HID);

    uint64_t tmp[1];
    tmp[0] = 1;
    status = H5Dwrite(dataset, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, tmp);
    assert(status >= 0);

    /* 
    create h5 space for storing the concatenated string.
    */
    dataset = H5Dcreate(h5.getId(), (dataset_name + "_$_str").c_str(), H5T_NATIVE_CHAR, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
    assert (dataset != H5I_INVALID_HID);
    char tmp_ch[1];
    tmp_ch[0] = '$';
    status = H5Dwrite(dataset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, tmp_ch);
    assert(status == 0);

    status = H5Pclose (cparms);
    assert(status == 0);
    status = H5Dclose (dataset);
    assert(status == 0);
    status = H5Sclose (dataspace);
    assert(status == 0);
}

std::vector<std::string> get_from_extendable_h5_dataset(uint64_t id_left, uint64_t id_right, const H5::Group &h5, const std::string &dataset_name) {
    if (id_right <= id_left) {
        Logger::error("Bad call to get string from hdf5 (left >= right): left=" + std::to_string(id_left) + " and right=" + std::to_string(id_right));
    }
    int64_t RANK = 1;

    hid_t pos_dset = H5Dopen (h5.getId(), (dataset_name + "_$_pos").c_str(), H5P_DEFAULT);
    hid_t pos_space = H5Dget_space (pos_dset);

    hsize_t pos_dimsf[RANK];
    int64_t ndims = H5Sget_simple_extent_dims (pos_space, pos_dimsf, NULL); 
    if (ndims == -1) {
        Logger::error("Dataset '" + dataset_name + "' is not properly initialised. Please make sure that you have already created an extendable dataset with this name.");
    }  
    assert(ndims == RANK);

    if (id_right >= pos_dimsf[0]) {
        Logger::error("internal error: request id that exceds the current size of extandable dataset");
    }

    /*
        select hyperslab from where to read:
        we need to get a block of 2 consecutive indices postions for having the starting index and the length of the target string.
    */
    herr_t status = H5Sselect_all (pos_space);
    assert(status == 0);
    hsize_t start[1]; start[0] = id_left;
    hsize_t count[1]; count[0] = id_right - id_left + 1;
    status = H5Sselect_hyperslab (pos_space, H5S_SELECT_SET, start, NULL, count, NULL);
    assert(status == 0);

    /*
     * select hyperslab for specifing the place inside "long long *rdata" where to put what is read (mandatory for mentioning rdata[0] and rdata[1], 
       otherwise there would be something like rdata[id], rdata[id+1]).
     */
    hsize_t memdims[1] = {id_right - id_left + 1};
    start[0] = 0;
    count[0] = id_right - id_left + 1;
    hid_t memspace = H5Screate_simple (1, memdims, NULL);
    status = H5Sselect_hyperslab (memspace, H5S_SELECT_SET, start, NULL, count, NULL);
    assert(status == 0);

    /*
        the proper reading from pos
    */
    long long *rdata =  (long long *) malloc ((id_right - id_left + 1) * sizeof (long long));
    rdata[0] = -1;
    status = H5Dread (pos_dset, H5T_NATIVE_LLONG, memspace, pos_space, H5P_DEFAULT, rdata);
    assert(status == 0);

    assert(rdata[id_right - id_left] - rdata[0] >= 0);

    std::vector<std::string> answer;
    if (rdata[id_right - id_left] == rdata[0]) {
        for (uint64_t i = id_left; i < id_right; ++i) {
            answer.push_back("");
        }
        free(rdata);
        return answer;
    }

    status = H5Dclose (pos_dset);
    assert(status == 0);
    status = H5Sclose (pos_space);
    assert(status == 0);
    status = H5Sclose (memspace);
    assert(status == 0);

    /*
     Next, we need to read and return the characters from 'dataset_name + "_$_str"'[rdata[0] : rdata[1]];
    */
    hid_t str_dset = H5Dopen (h5.getId(), (dataset_name + "_$_str").c_str(), H5P_DEFAULT);
    hid_t str_space = H5Dget_space (str_dset);

    hsize_t str_dimsf[RANK];
    ndims = H5Sget_simple_extent_dims (str_space, str_dimsf, NULL);    
    assert(ndims == RANK);

    /*
        select hyperslab from where to read:
    */
    status = H5Sselect_all (str_space);
    assert(status == 0);
    start[0] = rdata[0];
    count[0] = rdata[id_right - id_left] - rdata[0];
    status = H5Sselect_hyperslab (str_space, H5S_SELECT_SET, start, NULL, count, NULL);
    assert(status == 0);

    /*
     * select hyperslab for 'char *str_data ' (mandatory for making sure that the data is assigned to the prefix of str_data: str_data[0], str_data[1], str_data[2] etc.):
     */
    memdims[0] = rdata[id_right - id_left] - rdata[0];
    start[0] = 0;
    count[0] = memdims[0];
    memspace = H5Screate_simple (1, memdims, NULL);
    status = H5Sselect_hyperslab (memspace, H5S_SELECT_SET, start, NULL, count, NULL);
    assert(status == 0);

    /*
        the proper reading from str
    */
    char *str_data =  (char *) malloc ((rdata[id_right - id_left] - rdata[0]) * sizeof (char));
    status = H5Dread (str_dset, H5T_NATIVE_CHAR, memspace, str_space, H5P_DEFAULT, str_data);
    assert(status == 0);

    
    for (uint64_t id = id_left; id < id_right; ++id) {
        std::string curr;
        for (int64_t i = rdata[id - id_left] - rdata[0]; i < rdata[id - id_left + 1] - rdata[0]; ++i) {
            curr += str_data[i];
        }
        
        answer.push_back(curr);
        curr.clear();
    } 
    
    free(rdata);
    free(str_data);
    status = H5Dclose (str_dset);
    assert(status == 0);
    status = H5Sclose (str_space);
    assert(status == 0);
    status = H5Sclose (memspace);
    assert(status == 0);

    return answer;
}

void append_extendable_h5_dataset(const std::vector<std::string> &elems, H5::Group &h5, const std::string &dataset_name) {
    if (elems.empty()) {
        return;
    }

    // example: https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5-examples/browse/1_10/C/H5D/h5ex_d_unlimadd.c
    // example2:https://www.asc.ohio-state.edu/wilkins.5/computing/HDF/hdf5tutorial/examples/C/h5_hyperslab.c 
    int64_t RANK = 1;
    hsize_t str_dimsf[RANK];

    hid_t str_dset = H5Dopen (h5.getId(), (dataset_name + "_$_str").c_str(), H5P_DEFAULT);

    /*
     * Get dataspace and allocate memory for read buffer.  This is a
     * two dimensional dataset so the dynamic allocation must be done
     * in steps.
     */
    hid_t str_space = H5Dget_space (str_dset);

    /* 
      saving space dimensions into str_dimsf.
    */
    int64_t ndims = H5Sget_simple_extent_dims (str_space, str_dimsf, NULL);
    if (ndims == -1) {
        Logger::error("Dataset '" + dataset_name + "' is not properly initialised. Please make sure that you have already created an extendable dataset with this name.");
    }
    assert(ndims == RANK);

    /* 
        compose the two objects which will be further encoded and appended:
         * a concatenated string for the received strings
         * an integer vector for all the indices of the starting positions.
    */
    uint64_t merged_str_pos[elems.size()];
    std::string concat_str;
    for (uint64_t i = 0; i < elems.size(); ++i) {
        concat_str += elems[i];
        merged_str_pos[i] = str_dimsf[0] + concat_str.size();
    }

    /* 
    --------------------------------------------------------------------------------------------------------------------
            appending the list of starting indices
    --------------------------------------------------------------------------------------------------------------------
    */

    hid_t pos_dset = H5Dopen (h5.getId(), (dataset_name + "_$_pos").c_str(), H5P_DEFAULT);
    hsize_t pos_dimsf[RANK];
    /*
     * Get dataspace and allocate memory for read buffer.  This is a
     * two dimensional dataset so the dynamic allocation must be done
     * in steps.
     */
    hid_t pos_space = H5Dget_space (pos_dset);
    herr_t status = H5Sselect_all (pos_space);
    assert(status == 0);

    /* 
     * saving space dimensions into pos_dimsf.
     */
    ndims = H5Sget_simple_extent_dims (pos_space, pos_dimsf, NULL);    
    assert(ndims == RANK);
    
    status = H5Sclose (pos_space);
    assert(status == 0);

    /*
     * Extending the dataset.
     */
    hsize_t extdims[RANK] = {pos_dimsf[0] + elems.size()};

    status = H5Dset_extent (pos_dset, extdims);
    assert(status == 0);

    /*
     * Retrieving the dataspace for the newly extended dataset.
     */
    pos_space = H5Dget_space (pos_dset);

    /*
     * Select the entire dataspace.
     */
    status = H5Sselect_all (pos_space);
    assert(status == 0);

    /*
     * Subtract a hyperslab reflecting the original dimensions from the
     * selection.  The selection now contains only the newly extended
     * portions of the dataset.
     */
    hsize_t start[1], count[1], stride[1], block[1];
    start[0] = pos_dimsf[0];
    stride[0] = elems.size();
    count[0] = 1;
    block[0] = elems.size();
    status = H5Sselect_hyperslab (pos_space, H5S_SELECT_SET, start, stride, count, block);
    assert(status == 0);

    /*
     * Initialize data for writing to the extended dataset.
     */
    hsize_t memdims[RANK] = {elems.size()};
    start[0] = 0;
    count[0] = elems.size();
    hid_t memspace = H5Screate_simple (RANK, memdims, NULL);
    status = H5Sselect_hyperslab (memspace, H5S_SELECT_SET, start, NULL, count, NULL);
    assert(status == 0);

    /*
     * Write the data to the selected portion of the dataset.
     */
    status = H5Dwrite (pos_dset, H5T_NATIVE_LLONG, memspace, pos_space, H5P_DEFAULT, merged_str_pos);
    assert(status == 0);

    /*
     * Close and release resources.
     */
    status = H5Dclose (pos_dset);
    assert(status == 0);
    status = H5Sclose (pos_space);
    assert(status == 0);

    /* 
    --------------------------------------------------------------------------------------------------------------------
            appending the concatenated string
    --------------------------------------------------------------------------------------------------------------------
    */
    if (concat_str.size() == 0) {
        return;
    }


    /*
     * Extend the dataset.
     */
    extdims[0] = str_dimsf[0] + concat_str.size();

    status = H5Dset_extent (str_dset, extdims);
    assert(status == 0);

    /*
        select hyperslab where to write:
    */
    str_space = H5Dget_space (str_dset);
    status = H5Sselect_all (str_space);
    assert(status == 0);
    start[0] = str_dimsf[0];
    stride[0] = concat_str.size();
    count[0]  = 1;
    block[0] = concat_str.size();
    status = H5Sselect_hyperslab (str_space, H5S_SELECT_SET, start, stride, count, block);
    assert(status == 0);

    /*
        select hyperslab for transfering data from concat_str
    */
    memdims[0] = concat_str.size();
    start[0] = 0;
    count[0] = concat_str.size();
    memspace = H5Screate_simple (RANK, memdims, NULL);
    status = H5Sselect_hyperslab (memspace, H5S_SELECT_SET, start, NULL, count, NULL);
    assert(status == 0);

    /*
     * Append(write) concat_str data:
    */
    char concat_vchar[concat_str.size()] = {};
    for (uint64_t i = 0; i < concat_str.size(); ++i) {
        concat_vchar[i] = concat_str[i];
    }

    status = H5Dwrite (str_dset, H5T_NATIVE_CHAR, memspace, str_space, H5P_DEFAULT, concat_vchar);
    assert(status == 0);

    /*
     * Close and release resources.
     */
    status = H5Dclose (str_dset);
    assert(status == 0);
    status = H5Sclose (str_space);
    assert(status == 0);
}

void write_h5_dataset(const std::vector<std::string> &elems, H5::Group *h5, const std::string &dataset_name) {
    uint64_t merged_str_pos[elems.size()];
    std::string concat_str;

    for (uint64_t i = 0; i < elems.size(); ++i) {
        concat_str += elems[i];
        merged_str_pos[i] = concat_str.size();
    }
    write_posstr_to_h5_dataset(elems.size(), merged_str_pos, concat_str, h5, dataset_name);
}

std::vector<std::string> read_h5_dataset(const H5::Group &h5, const std::string &dataset_name) {
    uint64_t elem_size;
    uint64_t *positions;
    std::string loaded_str;
    read_posstr_to_h5_dataset(h5, dataset_name, &elem_size, positions, &loaded_str);
    std::vector<std::string> answer;
    if (elem_size != 0) {
        answer.push_back(loaded_str.substr(0, positions[0]));
        for (uint64_t i = 1; i < elem_size; ++i) {
            answer.push_back(loaded_str.substr(positions[i-1], positions[i] - positions[i-1]));
        }
    }
    delete positions;
    return answer;
}

template <typename T>
void write_h5_int_to_dataset(const std::vector<T> elems, H5::Group *h5, const std::string &dataset_name) {
    if(h5->attrExists(dataset_name.c_str())) {
        h5->removeAttr(dataset_name.c_str());
    }

    uint64_t RANK = 2;
    hsize_t dimsf[RANK];

    /* 
    write positions to hdf5
    */

    dimsf[0] = 1;
    dimsf[1] = elems.size();

    hid_t dataspace = H5Screate_simple(RANK, dimsf, NULL);
    hid_t datatype;
    
    if constexpr (std::is_same_v<T, uint64_t>) {
        datatype = H5Tcopy(H5T_STD_U64LE);
    } 
    if constexpr (std::is_same_v<T, int64_t>) {
        datatype = H5Tcopy(H5T_STD_I64LE);
    }

    T *elems_carray = new T[elems.size()];
    for (uint64_t i = 0; i < elems.size(); ++i) {
        elems_carray[i] = elems[i];
    }

    hid_t dataset = H5Dcreate(h5->getId(), dataset_name.c_str(), datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, elems_carray);
    assert(status >= 0);

    delete elems_carray;
    H5Tclose(datatype); 
    H5Dclose(dataset); 
    H5Sclose(dataspace);
}

template
void write_h5_int_to_dataset(const std::vector<uint64_t> elems, H5::Group *h5, const std::string &dataset_name);

template
void write_h5_int_to_dataset(const std::vector<int64_t> elems, H5::Group *h5, const std::string &dataset_name);

template <typename T>
std::vector<T> read_h5_int_to_dataset(const H5::Group &h5, const std::string &dataset_name) {
    hsize_t dims_out[2];
    hsize_t max_dims[2];
    
    /* 
        read positions dataset
    */
    hid_t dataset = H5Dopen(h5.getId(), dataset_name.c_str(), H5P_DEFAULT);
    hid_t datatype  = H5Dget_type(dataset);
    hid_t dataspace = H5Dget_space(dataset);
    
    int rank = H5Sget_simple_extent_ndims(dataspace);
    assert (rank == 2);
    int status_n  = H5Sget_simple_extent_dims(dataspace, dims_out, max_dims);
    assert(status_n >= 0);

    T *elems_carray = new T[dims_out[0] * dims_out[1]];
    herr_t status = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, elems_carray);
    assert(status >= 0);

    std::vector<T> answer;
    for (uint64_t i = 0; i < dims_out[1]; ++i) {
        answer.push_back(elems_carray[i]);
    }

    delete elems_carray;
    H5Dclose(dataset);
    H5Tclose(datatype);
    H5Sclose(dataspace);

    return answer;
}

template
std::vector<uint64_t> read_h5_int_to_dataset(const H5::Group &h5, const std::string &dataset_name);

template
std::vector<int64_t> read_h5_int_to_dataset(const H5::Group &h5, const std::string &dataset_name);

} // namespace H5Helper