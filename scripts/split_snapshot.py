#!/usr/bin/python
# Taken from https://www.tutorialspoint.com/python/python_command_line_arguments.htm

# This script is used to split a snapshot json in multiple small modules. All of these modules will be aligned in parallel. 

# Usage example:
#       python split_snapshot.py -i 2021-06-27.provision.json -o tmp/aux_align -n 128 --lookup_hash_align <file.xz>

# Will create 128 small which sum up to the initial "2021-06-27.provision.json".
# For each module there will be 2 files created:
#  #  x.fasta which is used by nextalign
#  #  x.noseq_provision.json which will be further used to store the same json snapshot format for the aligned sequence

# The additional 'lookup_align.json.xz' file contains a lookup from sequence hashes (in sha256 format) to sequence alignment content (string arrays of size ~40000). 
#    This file will be used for improving the runtime by not aligning the same sequence multiple times.

import constants

import sys, getopt, json, os, subprocess

def print_helper():
    print ('split_snapshot.py -i <inputfile> -o <outputpath> -n <num_files> --lookup_hash_align <file.xz>')

def main(argv):
    inputfile = ''
    outputdir = ''
    num_files_str = ''
    lookup_hash_align_filepath = ''
    try:
        opts, _ = getopt.getopt(argv,"hi:o:n:",["ifile=", "ofile=", "numfiles=", "lookup_hash_align="])
    except getopt.GetoptError:
        print_helper()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('test.py -i <inputfile> -o <outputfile> -n <num_files>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputdir = arg
        elif opt in ("-n", "--numfiles"):
            num_files_str = arg
        elif opt in "--lookup_hash_align":
            lookup_hash_align_filepath = arg

    if (not num_files_str.isnumeric()):
        print ("Error: received invalid number of output files '", num_files_str, "'")
        print_helper()
        sys.exit(2)

    if (not lookup_hash_align_filepath.endswith(".json.xz")):
        print ("The provided lookup_hash_align filepath is a not '.json.xz' compressed file '" + lookup_hash_align_filepath + "'.")
        print_helper()
        sys.exit(2)

    # Create temporary directory:
    if (not outputdir.endswith("/")):
        outputdir = outputdir + "/"
    os.mkdir(outputdir)

    # Decompress 'lookup_hash_align_filepath' to the temporary location:

    if not os.path.isfile(lookup_hash_align_filepath):
        print ("Provided filepath '" + lookup_hash_align_filepath + "' does not exist")
    os.system("cp " + lookup_hash_align_filepath + " " + outputdir)
    p = subprocess.Popen(["unxz", os.path.basename(lookup_hash_align_filepath)], cwd=outputdir)
    p.wait()
    if p.poll() != 0:
        print ("error while applying unxz on the compressed file " + lookup_hash_align_filepath)
        sys.exit(2)

    # Rename the file:
    os.system("mv " + outputdir + os.path.basename(lookup_hash_align_filepath).rstrip(".xz") + " " + outputdir + constants.LOOKUP_ALIGN_BASENAME)

    num_files = int(num_files_str)

    print ('Input file is "', inputfile, '"')
    print ('Output dir is "', outputdir, '"')
    print ('num_files =', num_files)

    output_fasta = []
    output_json = []

    for i in range(num_files):
        f = open(outputdir + str(i) + ".fasta", "w")
        output_fasta.append(f)

        f = open(outputdir + str(i) + ".noseq_provision.json", "w")
        output_json.append(f)

    already_aligned_file = open(outputdir + "prv_aligned.provision.json", "w")
    lookup_align_hashes = constants.get_hash_lookup(outputdir + constants.LOOKUP_ALIGN_BASENAME)

    num_seq_to_skip = 0
    num_seq_to_align = 0
    cnt = 0
    for line in open(inputfile, 'r'):
        json_seq = json.loads(line)

        curr_hash = constants.get_hash(json_seq["sequence"]) 
        json_seq["seq_hash"] = curr_hash

        if curr_hash in lookup_align_hashes:
            num_seq_to_skip = num_seq_to_skip + 1
            json_seq["sequence"] = lookup_align_hashes[curr_hash]
            json.dump(json_seq, already_aligned_file)
            already_aligned_file.write("\n")
            continue

        output_fasta[cnt].write(">sequence " + json_seq["covv_accession_id"] + " \n")
        output_fasta[cnt].write(json_seq["sequence"] + "\n")

        json_seq["sequence"] = "?"
        json.dump(json_seq, output_json[cnt])
        output_json[cnt].write("\n")

        cnt = (cnt + 1) % num_files
        num_seq_to_align = num_seq_to_align + 1
    print("Number of skiped sequences", num_seq_to_skip)
    print("Number of sequences to align", num_seq_to_align)

if __name__ == "__main__":
   main(sys.argv[1:])
