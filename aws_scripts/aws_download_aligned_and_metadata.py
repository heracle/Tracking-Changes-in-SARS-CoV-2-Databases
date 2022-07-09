#!/usr/bin/python

metadata_tsv_url = "https://nextstrain-data.s3.amazonaws.com/?versions&prefix=files/ncov/open/metadata.tsv.gz"
aligned_tsv_url = "https://nextstrain-data.s3.amazonaws.com/?versions&prefix=files/ncov/open/aligned.fasta.xz"
metadata_version_url_prefix = "http://nextstrain-data.s3.amazonaws.com/files/ncov/open/metadata.tsv.gz?versionId="
aligned_version_url_prefix = "http://nextstrain-data.s3.amazonaws.com/files/ncov/open/aligned.fasta.xz?versionId="

# TODO add description

from ast import arg
import sys, getopt, csv, json, io
import subprocess
import gzip, os, csv
import pandas as pd

import json
import requests

try:
    import lzma
except ImportError:
    from backports import lzma

def print_helper():
    print ("""
    This script should be run in the following way:
    
    python3 aws_download_aligned_and_metadata.py <timestamp> <region> <country> <tmp_directory>

    example: python3 aws_download_aligned_and_metadata.py '2021-07-09T08:05:47.000Z' 'Europe' 'United Kingdom' '/cluster/scratch/rmuntean/gisaid_data/tmp/'
""")

def get_versionId_older_than_modifiedTag(tsv_url, reqLastModifiedTag):
    metadata = requests.get(tsv_url).text

    metadata_splitted = metadata.split("<VersionId>")

    for i in range(len(metadata_splitted)):
        if (i == 0):
            # just ignore the header line
            continue
        curr = metadata_splitted[i]

        curr_LastModified = curr.split("<LastModified>")[1].split("</LastModified>")[0]

        if (curr_LastModified <= reqLastModifiedTag):
            return curr.split("</VersionId>")[0]
    print ("There are no versions older than " + reqLastModifiedTag);

def main(argv):
    if len(argv) != 4:
        print_helper()
        exit()

    reqLastModifiedTag = argv[0]
    reqRegion = argv[1]
    reqCountry = argv[2]
    tmpDir= argv[3]
    
    print ('reqLastModifiedTag file is "', reqLastModifiedTag, '"')
    print ('reqRegion file is "', reqRegion, '"')
    print ('reqCountry =', reqCountry)
    print ('tmpDir =', tmpDir)

    # Reading the metadata tsv.gz file with all the version IDs.
    metadata_version_id = get_versionId_older_than_modifiedTag(metadata_tsv_url, reqLastModifiedTag)

    # Reading the aligned tsv.gz file with all the version IDs.
    aligned_version_id = get_versionId_older_than_modifiedTag(aligned_tsv_url, reqLastModifiedTag)

    print("metadata_version_id= " + metadata_version_id)
    print("aligned_version_id= " + aligned_version_id)

    # Get metadata from the specific version
    gz_metadata = requests.get(metadata_version_url_prefix + metadata_version_id).text
    pd_metadata = pd.read_csv(io.StringIO(gz_metadata), sep='\t')
    strain_to_aligned = {}
    for index, row in pd_metadata.iterrows():
        if "EPI_ISL_" not in row['gisaid_epi_isl']:
            continue
        if not row['region'].startswith(reqRegion):
            continue
        if not row['country'].startswith(reqCountry):
            continue
        strain_to_aligned[str(row['strain'])] = ""

    # Get aligned from the specific version
    subprocess.run(["wget", aligned_version_url_prefix + aligned_version_id, "-O", tmpDir + "aligned.fasta.xz"])
    subprocess.run(["unxz", tmpDir + "aligned.fasta.xz", "--force"])

    curr_seq_strain = ""
    with open(tmpDir + "aligned.fasta") as infile:
        for line in infile:
            if line.startswith(">"):
                curr_seq_strain = line[1:len(line) - 1]
                curr_seq_strain.strip()
            else:
                if curr_seq_strain in strain_to_aligned:
                    strain_to_aligned[curr_seq_strain] = line[:len(line) - 1].strip()
                
    # Create the final json
    output_json_path = tmpDir + "output.json"
    output_json = open(output_json_path, "w")

    for index, row in pd_metadata.iterrows():
        seq_json = {}
        strain = str(row['strain'])
        if (strain not in strain_to_aligned) or (strain_to_aligned[strain] == ""):
            continue
        seq_json["covv_accession_id"] = row["gisaid_epi_isl"]
        seq_json["covv_collection_date"] = row["date"]
        seq_json["covv_location"] = row["region"] + " / " + row["country"] + " / " + row["division"]
        seq_json["sequence"] = strain_to_aligned[strain]
        seq_json["owner"] = row["authors"]
        seq_json["covv_host"] = row["host"]
        json.dump(seq_json, output_json)
        output_json.write("\n")
    
    print("\nThe content was written at filepath '" + output_json_path + "'.")

if __name__ == "__main__":
   main(sys.argv[1:])