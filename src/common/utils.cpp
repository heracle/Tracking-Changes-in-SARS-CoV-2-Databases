#include "utils.hpp"

#include <fstream>

#include "constants.hpp"
#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"

#include "../external_libraries/json.hpp"
using Json = nlohmann::json;

namespace common {

int Tnode::next_rand_idx = rand_pool_size;
std::vector<unsigned long long> Tnode::rand_values = std::vector<unsigned long long>(Tnode::rand_pool_size);

Tnode::Tnode(const int index) : data_id(index), prio(get_rand_ull()), l(NULL), r(NULL) {
    // todo decide if we need to compute mutation?
};
Tnode::Tnode(const Tnode &oth) : data_id(oth.data_id), prio(oth.prio), l(oth.l), r(oth.r) /*, total_mutations(oth.total_mutations), mutation_freq(oth.mutation_freq)*/ {}; // todo move these mutations to the constructor of the derived class.
Tnode::Tnode(const H5::Group &tnode_group) {
    this->data_id = H5Helper::get_uint32_attr_from(tnode_group, "data_id");
    this->prio = H5Helper::get_uint64_attr_from(tnode_group, "prio");

    this->l = NULL;
    this->r = NULL;

    // this->total_mutations = 0;
    // for (int i = 0; i < proto.mutations_freq_key_size(); ++i) {
    //     this->mutation_freq[proto.mutations_freq_key(i)] = proto.mutations_freq_value(i); 
    //     this->total_mutations += proto.mutations_freq_value(i);
    // }
}

unsigned long long Tnode::get_rand_ull() {
    if (next_rand_idx == rand_pool_size) {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> unif_dist(1, 0xFFFFFFFFFFFFFFFF);
        for (unsigned int i = 0; i < rand_pool_size; ++i) {
            rand_values[i] = unif_dist(rng);
        }
        next_rand_idx = 0;
    }
    return rand_values[next_rand_idx++];
}

SeqElem::SeqElem() {}

SeqElem SeqElem::get_from_hdf5(const H5::Group &group) {
    SeqElem curr;
    curr.covv_accession_id = H5Helper::get_string_attr_from(group, "covv_accession_id");
    curr.covv_collection_date = H5Helper::get_string_attr_from(group, "covv_collection_date");
    curr.covv_location = H5Helper::get_string_attr_from(group, "covv_location");
    curr.sequence = H5Helper::get_string_attr_from(group, "sequence");;
    return curr;
}

uint64_t get_hash(std::string s, uint64_t hash) {
    for (const auto c : s) {
        hash = (hash * 117 + c) % common::MOD1;
    }
    return hash;
}

common::SeqElem get_SeqElem_from_json(Json j_obj) {
    common::SeqElem answer;
    answer.covv_accession_id = j_obj["covv_accession_id"];
    answer.covv_collection_date = j_obj["covv_collection_date"];
    answer.covv_location = j_obj["covv_location"];
    answer.sequence = j_obj["sequence"];
    answer.covsurver_prot_mutations = j_obj["covsurver_prot_mutations"];
    if (answer.covsurver_prot_mutations == "") {
        // Otherwise get an error on sequence EPI_ISL_412860.
        answer.covsurver_prot_mutations = "()";
    }
    return answer;
}

std::vector<common::SeqElem> get_aligned_seq_elements(const std::string &input_path) {
    Json j_obj;
    Logger::trace("Getting sequence and metadata from file '" + input_path + "'...");
    std::ifstream f(input_path, std::ifstream::binary);

    // Create a temporary file to save all sequences to align.
    // std::string fasta_tmp = "/cluster/scratch/rmuntean/gisaid_data/tmp3";
    // uint32_t num_sequences = 0;
    // std::ofstream fout_fasta (fasta_tmp + ".fasta");
    std::vector<common::SeqElem> answer;

    // tsl::hopscotch_map<std::string, common::SeqElem> seq_elems;
    while (true) {
        try {
            f >> j_obj;
        }
        catch (Json::exception& e) {
            // todo - more precise error catch
            break;
        }

        common::SeqElem seqelem_val = common::get_SeqElem_from_json(j_obj);

        // todo activate nextalign!
        answer.push_back(seqelem_val);

        // fout_fasta << ">sequence " + seqelem_val.covv_accession_id << "\n";
        // fout_fasta << seqelem_val.sequence << "\n";

        // seq_elems[seqelem_val.covv_accession_id] = seqelem_val;
        // ++num_sequences;
    }
    // fout_fasta.close();
    f.close();

    // // Run nextalign agains the temporary file
    // std::string nextalign_cmd = common::NEXTALIGN_EXE_PATH + " run -i " + fasta_tmp + ".fasta --input-dataset " + common::COVID_NEXTALIGN_DATASET + " --output-basename " + fasta_tmp;
    // system(nextalign_cmd.c_str());

    // // Open the output file and update seq_elems accoring to the aligned conterpart.
    // std::ifstream fin_fasta (fasta_tmp + ".aligned.fasta");


    // std::string dev_null, seq_acc_version, seq;
    // for (uint32_t i = 0; i < num_sequences; ++i) {
    //     if (!(fin_fasta >> dev_null >> seq_acc_version >> seq)) {
    //         Logger::warn("There are sequences missing in the alignment file.");
    //         break;
    //     }
        
    //     if (seq_elems.count(seq_acc_version) == 0) {
    //         Logger::error("Can't find the aligned sequence for acc_id " + seq_acc_version);
    //     }
    //     auto &seq_elem = seq_elems[seq_acc_version];
    //     seq_elem.sequence = seq;
    //     answer.push_back(seq_elem);
    // }
    // fin_fasta.close();
    return answer;
}

std::vector<std::string> get_mutations_from_json_str(const std::string &mutation_str) {
    std::vector<std::string> answer;

    // we need to save the mutations for this current tnode.
    std::string curr_mutation = "";
    // mutation string contains brackets, e.g. "(NSP15_A283V,NSP12_P323L,Spike_D614G)".
    for (uint32_t i = 1; i < mutation_str.size() - 1; ++i) {
        if (mutation_str[i] == ',') {
            answer.push_back(curr_mutation);
            curr_mutation.clear();
        } else {
            curr_mutation.push_back(mutation_str[i]);
        }
    }
    // add last mutation.
    if (curr_mutation != "") {
        answer.push_back(curr_mutation);
    }

    return answer;
}

} // namespace common
