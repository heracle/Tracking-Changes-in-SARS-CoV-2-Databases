#!/usr/bin/python
# # Taken from https://www.tutorialspoint.com/python/python_command_line_arguments.htm

# This script merges together all the aligned chunked files.

# If lookup_output_filepath not empty: the script will save the final aligned sequences file to the 'outputpath.provision.json' and the updated lookup table to the 'lookup_outputpath.xz'.

import sys, getopt, json, subprocess, os
import constants
import lzma as xz

def print_helper():
    print ('merge_aligned_modules.py -i <inputdir> -o <outputpath.provision.json> -n <num_files> --lookup_hash_align <lookup_outputpath.xz>')

def main(argv):
    inputdir = ''
    outputfile = ''
    num_files_str = ''
    lookup_output_filepath = ""
    try:
        opts, _ = getopt.getopt(argv,"hi:o:n:",["idir=", "ofile=", "numfiles=", "lookup_output="])
    except getopt.GetoptError:
        print_helper()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print_helper()
            sys.exit()
        elif opt in ("-i", "--idir"):
            inputdir = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
        elif opt in ("-n", "--numfiles"):
            num_files_str = arg
        elif opt in "--lookup_output":
            lookup_output_filepath = arg

    if (not num_files_str.isnumeric()):
        print ("Error: received invalid number of output files '", num_files_str, "'")
        print_helper()
        sys.exit(2)

    num_files = int(num_files_str)

    print ('Input dir is "', inputdir, '"')
    print ('Output file is "', outputfile, '"')
    print ('num_files =', num_files)

    if inputdir == "":
        print ("Error: inputdir cannot be an empty path.")
        print_helper()
        sys.exit(2)
    
    if outputfile == "":
        print ("Error: output cannot be an empty path.")
        print_helper()
        sys.exit(2)

    if not outputfile.endswith(".provision.json"):
        # outputfile = outputfile + ".provision.json"
        print ("Error: filepath to save the output should have a '.provision.json' format.")
        print_helper()
        sys.exit(2)
    
    if lookup_output_filepath != "" and (not lookup_output_filepath.endswith(".json.xz")):
        print ("Error: filepath to save the lookup hash alignment should have a '.json.xz' format.")
        print_helper()
        sys.exit(2)
    
    if not inputdir.endswith("/"):
        inputdir = inputdir + "/"
    
    print("outputfile:", outputfile)

    # Append the sequences which were aligned via the lookup table.    
    os.system("mv " + inputdir + "prv_aligned.provision.json " + outputfile)
    output = open(outputfile, "a")

    if lookup_output_filepath != "":
        lookup_align_hashes = constants.get_hash_lookup(constants.LOOKUP_ALIGN_BASENAME)
    
    for i in range(num_files):
        cnt_per_file = 0

        input_filepath = inputdir + "aligned" + str(i) + ".provision.json"

        if os.stat(input_filepath).st_size == 0:
            print("file '" + input_filepath + "' has size 0, ignoring..")
            continue

        for line in open(input_filepath, "r"):
            cnt_per_file = cnt_per_file + 1
            output.write(line + "\n")

            if line == "":
                continue
            # update the lookup table
            json_seq = json.loads(line)

            if lookup_output_filepath != "":
                lookup_align_hashes[json_seq["seq_hash"]] = json_seq["sequence"]
    
    if lookup_output_filepath != "":
        new_lookup_file = open(inputdir + "modified_lookup.json", "w")
        json.dump(lookup_align_hashes, new_lookup_file, indent=2)
        new_lookup_file.write("\n")
        new_lookup_file.close()
        os.system("xz " + inputdir + "modified_lookup.json")
        os.system("cp " + inputdir + "modified_lookup.json.xz " + lookup_output_filepath)

if __name__ == "__main__":
   main(sys.argv[1:])
