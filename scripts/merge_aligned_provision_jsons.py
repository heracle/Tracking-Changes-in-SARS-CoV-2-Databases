# Taken from https://www.tutorialspoint.com/python/python_command_line_arguments.htm

#!/usr/bin/python

import sys, getopt

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

    if inputfile == "":
        print ("Error: inputfile cannot be an empty path.")
        print ('test.py -i <inputfile> -o <outpupath> -n <num_files>')
        sys.exit(2)
    
    if outputfile == "":
        print ("Error: output cannot be an empty path.")
        print ('test.py -i <inputfile> -o <outpupath> -n <num_files>')
        sys.exit(2)

    if not outputfile.endswith(".provision.json"):
        outputfile = outputfile + ".provision.json"
    
    print("outputfile:", outputfile)
    output = open(outputfile, "w")

    for i in range(num_files):        
        for line in open(inputfile + str(i) + ".provision.json", "r"):
            output.write(line + "\n")

if __name__ == "__main__":
   main(sys.argv[1:])