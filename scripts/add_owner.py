#!/usr/bin/python

# This script adds the 'owner' field as one GISAID snapshot preprocessing step.

import sys, getopt, csv, json


def print_helper():
    print ('add_owner.py -i <inputpath.provision.json> -o <outputpath.provision.json> --submitter-info <submitter_information.csv>')

def main(argv):
    inputfile = ''
    outputfile = ''
    submitter_filepath = ''
    try:
        opts, _ = getopt.getopt(argv,"hi:o:",["idir=", "ofile=", "submitter-info="])
    except getopt.GetoptError:
        print_helper()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print_helper()
            sys.exit()
        elif opt in ("-i", "--idir"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
        elif opt in "--submitter-info":
            submitter_filepath = arg
    
    print ('Input file is "', inputfile, '"')
    print ('Output file is "', outputfile, '"')
    print ('submitter_filepath =', submitter_filepath)

    if not inputfile.endswith(".provision.json"):
        print ("Error: inputfile must have a '.provision.json' format.")
        print_helper()
        sys.exit(2)
    
    if not outputfile.endswith(".provision.json"):
        print ("Error: outputfile must have a '.provision.json' format.")
        print_helper()
        sys.exit(2)

    if not submitter_filepath.endswith("_information.csv"):
        print ("Error: outputfile must have a '_information.csv' suffix.")
        print_helper()
        sys.exit(2)

    accid_to_owner = {}
    with open(submitter_filepath, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        cnt = 0
        for row in reader:
            accid_to_owner[row['gisaid_epi_isl']] = str(row['submitting_lab'])
            cnt = cnt + 1
    
    output_json = open(outputfile, "w")
    
    for line in open(inputfile, 'r'):
        if line == "" or line == "\n":
            continue
        json_seq = json.loads(line)
        curr_accid = str(json_seq["covv_accession_id"])
        if curr_accid in accid_to_owner:
            json_seq["owner"] = accid_to_owner[curr_accid]
        else:
            json_seq["owner"] = ""
        json.dump(json_seq, output_json)
        output_json.write("\n")

if __name__ == "__main__":
   main(sys.argv[1:])