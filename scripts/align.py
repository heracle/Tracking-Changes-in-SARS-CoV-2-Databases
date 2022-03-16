# Taken from https://www.tutorialspoint.com/python/python_command_line_arguments.htm

#!/usr/bin/python

import sys, getopt, json, os
import multiprocessing

NEXTALIGN_EXE_PATH = "../src/external_libraries/nextclade-Linux-x86_64"
COVID_NEXTALIGN_DATASET = "../sars_cov_dataset"

def run_file(i, inputfile, outputfile):
    input_fasta = inputfile + str(i) + ".fasta"
    output_fasta = inputfile + str(i) + ".aligned.fasta"

    nextalign_cmd = NEXTALIGN_EXE_PATH + " run -i " + input_fasta + " --input-dataset " + COVID_NEXTALIGN_DATASET + " --output-basename " + output_fasta;
    os.system(nextalign_cmd)

    input_json = inputfile + str(i) + ".noseq_provision.json"
    raw_json_data = {}

    for line in open(input_json, 'r'):
        json_seq = json.loads(line)
        raw_json_data[json_seq["covv_accession_id"]] = json_seq
        # print(json_seq["covv_accession_id"])

    output_json = open(outputfile + str(i) + ".provision.json", "w")

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
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv,"hi:o:n:",["ifile=", "ofile=", "numfiles="])
    except getopt.GetoptError:
        print ('test.py -i <inputfile> -o <outpupath> -n <num_files>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('test.py -i <inputfile> -o <outputfile> -n <num_files>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
        elif opt in ("-n", "--numfiles"):
            num_files_str = arg

    if (not num_files_str.isnumeric()):
        print ("Error: received invalid number of output files '", num_files_str, "'")
        print ('test.py -i <inputfile> -o <outpupath> -n <num_files>')
        sys.exit(2)

    num_files = int(num_files_str)

    print ('Input file is "', inputfile, '"')
    print ('Output file is "', outputfile, '"')
    print ('num_files =', num_files)

    pool_obj = multiprocessing.Pool(num_files)
    pool_args = []
    for i in range(num_files):
        curr_args = (i, inputfile, outputfile)
        pool_args.append(curr_args)

    pool_obj.starmap(run_file, pool_args)


if __name__ == "__main__":
   main(sys.argv[1:])


# bsub -W 24:00 -R "rusage[mem=3000]" -n 64 python align.py -i /cluster/scratch/rmuntean/gisaid_data/tmp_2021-06-27 -o /cluster/scratch/rmuntean/gisaid_data/2021-06-27_aligned.provision.json -n 64

# bsub -R "rusage[mem=20000]" python split_provision_json.py -i /cluster/scratch/rmuntean/gisaid_data/2021-06-27.provision.json -o /cluster/scratch/rmuntean/gisaid_data/tmp_2021-06-27 -n 64

# ls -lah tmp_2021-06*.aligned.fasta.aligned*

# (base) [rmuntean@eu-login-30 gisaid_data]$ ls -lah tmp_2021-06-270.fasta 
# -rw-r----- 1 rmuntean rmuntean-group 935M Jan  9 16:05 tmp_2021-06-270.fasta


# (base) [rmuntean@eu-login-30 gisaid_data]$ ls -lah tmp_2021-07-110.fasta 
# -rw-r----- 1 rmuntean rmuntean-group 1017M Jan  9 16:48 tmp_2021-07-110.fasta

# (base) [rmuntean@eu-login-15 scripts]$ python merge_aligned_provision_jsons.py -i /cluster/scratch/rmuntean/gisaid_data/2021-06-27_aligned.provision.json -o /cluster/scratch/rmuntean/gisaid_data/2021-06-27_aligned -n 64