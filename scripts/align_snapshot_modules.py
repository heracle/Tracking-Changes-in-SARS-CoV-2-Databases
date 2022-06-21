#!/usr/bin/python

# Taken from https://www.tutorialspoint.com/python/python_command_line_arguments.htm

# This script runs Nextalign against each snapshot module.

import constants

import sys, getopt, json, os
import multiprocessing

def print_helper():
    print ('align_snapshot_modules.py -d <working_dir> -n <num_files>')

# Run nextalign agains each snapshot module in parallel:
def run_file(i, w_dir):
    print("starting thread i=" + str(i))
    input_fasta = w_dir + str(i) + ".fasta"
    output_fasta = w_dir + "AL_" + str(i)

    nextalign_cmd = constants.NEXTALIGN_EXE_PATH + " run -i " + input_fasta + " --input-dataset " + constants.COVID_NEXTALIGN_DATASET + " --output-basename " + output_fasta;
    os.system(nextalign_cmd)

    input_json = w_dir + str(i) + ".noseq_provision.json"
    # raw_json_data is a dictionary where the key is an accession id and the value is the entire json from the snapshot file. 
    raw_json_data = {}

    output_json = open(w_dir + "aligned" + str(i) + ".provision.json", "w")

    if os.stat(input_json).st_size == 0:
        print("file '" + input_json + "' has size 0, ignoring..")
        output_json.append("")
        return

    for line in open(input_json, 'r'):
        json_seq = json.loads(line)
        raw_json_data[json_seq["covv_accession_id"]] = json_seq

    last_seqid = ""
    curr_sequence = ""
    for line in open(output_fasta + ".aligned.fasta", "r"):
        if not line.startswith(">sequence"):
            curr_sequence = curr_sequence + line
            continue
        if last_seqid != "":
            curr_json = raw_json_data[last_seqid]
            curr_json["sequence"] = curr_sequence
            json.dump(curr_json, output_json)
            output_json.write("\n")
        last_seqid = line.split(' ')[1]
        curr_sequence = ""
    
    if last_seqid != "":
        curr_json = raw_json_data[last_seqid]
        curr_json["sequence"] = curr_sequence
        json.dump(curr_json, output_json)    
        output_json.write("\n")

def main(argv):
    num_files_str = ''
    tmp_dir = ''
    try:
        opts, _ = getopt.getopt(argv,"hd:n:",["wdir=", "numfiles="])
    except getopt.GetoptError:
        print_helper()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print_helper()
            sys.exit()
        elif opt in ("-d", "--wdir"):
            tmp_dir = arg
        elif opt in ("-n", "--numfiles"):
            num_files_str = arg

    if (not tmp_dir.endswith("/")):
        tmp_dir = tmp_dir + "/"

    if (not num_files_str.isnumeric()):
        print ("Error: received invalid number of output files '", num_files_str, "'")
        print_helper()
        sys.exit(2)

    num_files = int(num_files_str)

    print ('Workind directory is "', tmp_dir, '"')
    print ('num_files =', num_files)

    pool_obj = multiprocessing.Pool(num_files)
    pool_args = []
    for i in range(num_files):
        curr_args = (i, tmp_dir)
        pool_args.append(curr_args)

    pool_obj.starmap(run_file, pool_args)


if __name__ == "__main__":
   main(sys.argv[1:])
