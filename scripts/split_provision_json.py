# Taken from https://www.tutorialspoint.com/python/python_command_line_arguments.htm

#!/usr/bin/python

import sys, getopt, json

def main(argv):
    inputfile = ''
    outputfile = ''
    num_files_str = ''
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

    output_fasta = []
    output_json = []

    for i in range(num_files):
        f = open(outputfile + str(i) + ".fasta", "w")
        output_fasta.append(f)

        f = open(outputfile + str(i) + ".noseq_provision.json", "w")
        output_json.append(f)

    cnt = 0
    for line in open(inputfile, 'r'):
        json_seq = json.loads(line)
        output_fasta[cnt].write(">sequence " + json_seq["covv_accession_id"] + " \n")
        output_fasta[cnt].write(json_seq["sequence"] + "\n")

        json_seq["sequence"] = "?"
        json.dump(json_seq, output_json[cnt])
        output_json[cnt].write("\n")

        cnt = (cnt + 1) % num_files

if __name__ == "__main__":
   main(sys.argv[1:])
