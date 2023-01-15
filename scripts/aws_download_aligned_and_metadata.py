#!/usr/bin/python

# This script is used to download SARS-CoV-2 sequences from amazon AWS detabank and
# save them in a JSON format compatible to the tracking changes tool.

import os
import gzip
import io
import json
import csv
import getopt
from urllib import request
import string
import random
import sys
import requests
from ast import arg
import shutil
import pandas as pd
import subprocess
import argparse
metadata_tsv_url = "https://nextstrain-data.s3.amazonaws.com/?versions&prefix=files/ncov/open/metadata.tsv.gz"
aligned_tsv_url = "https://nextstrain-data.s3.amazonaws.com/?versions&prefix=files/ncov/open/aligned.fasta.xz"
metadata_version_url_prefix = "http://nextstrain-data.s3.amazonaws.com/files/ncov/open/metadata.tsv.gz?versionId="
aligned_version_url_prefix = "http://nextstrain-data.s3.amazonaws.com/files/ncov/open/aligned.fasta.xz?versionId="


try:
    import lzma
except ImportError:
    from backports import lzma


def get_versionId_older_than_modifiedTag(tsv_url, reqLastModifiedTag):
    metadata = requests.get(tsv_url).text
    metadata_splitted = metadata.split("<VersionId>")
    for i in range(len(metadata_splitted)):
        if (i == 0):
            # Always ignore the header line.
            continue
        curr = metadata_splitted[i]
        curr_LastModified = curr.split("<LastModified>")[1].split("</LastModified>")[0]
        if (curr_LastModified <= reqLastModifiedTag):
            return curr.split("</VersionId>")[0]
    print("There are no versions older than " + reqLastModifiedTag)


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--date", required=True, type=str,
                        help="Get all sequences submitted before this date ('YEAR-MM-DD' format).")
    parser.add_argument("-l", "--limit", default=0, type=int,
                        help="Limit the number of returned sequences. A value less or equal than 0 means no limit enforced.")
    parser.add_argument("-r", "--region", default="", type=str,
                        help="Get only the sequences that contain this specific region prefix.")
    parser.add_argument("-c", "--country", default="", type=str,
                        help="Get only the sequences that contain this specific country prefix.")
    parser.add_argument("--tmp_dir", default=".",
                        help="Filepath to a tmp directory to save all the final and intermediary files.")
    parser.add_argument("-o", "--output", required=True,
                        help="Filepath to a tmp directory to save all the final and intermediary files.")
    args = parser.parse_args()

    # Run_id composed of 6 chars.
    run_id = ''.join(random.choices(string.ascii_uppercase + string.digits, k=6))
    args.output += '_' + args.date + '.json'

    # Reading the metadata tsv.gz file with all the version IDs.
    metadata_version_downloade_link = metadata_version_url_prefix + \
        get_versionId_older_than_modifiedTag(metadata_tsv_url, args.date)

    # Reading the aligned tsv.gz file with all the version IDs.
    aligned_version_download_link = aligned_version_url_prefix + \
        get_versionId_older_than_modifiedTag(aligned_tsv_url, args.date)

    print("metadata will be downloaded from " + metadata_version_downloade_link)
    print("aligned sequences will be downloaded from " + aligned_version_download_link)

    # Get metadata from the specific version
    metadata_raw_gz = os.path.join(args.tmp_dir, "__tmp__id_" + run_id + "__metadata_raw.tsv.gz")
    metadata_raw = os.path.join(args.tmp_dir, "__tmp__id_" + run_id + "__metadata_raw.tsv")
    request.urlretrieve(metadata_version_downloade_link, metadata_raw_gz)
    # Unzip metadata_raw_gz to metadata_raw.
    with gzip.open(metadata_raw_gz, 'rb') as f_in:
        with open(metadata_raw, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
    os.remove(metadata_raw_gz)

    strain_to_aligned = {}
    # Only `chunksize` rows will be read at once to reduce RAM usage.
    chunksize = 2 ** 10
    for chunk in pd.read_csv(metadata_raw, sep='\t', chunksize=chunksize):
        for _, row in chunk.iterrows():
            # Select only the sequences that are available on GISAID and have the specifiec region and country,
            if "EPI_ISL_" not in row['gisaid_epi_isl']:
                continue
            if not row['region'].startswith(args.region):
                continue
            if not row['country'].startswith(args.country):
                continue
            strain_to_aligned[str(row['strain'])] = ""
            if len(strain_to_aligned) == args.limit:
                break
        if len(strain_to_aligned) == args.limit:
            break

    print("There will be", len(strain_to_aligned), "sequences that are going to be saved.")

    # Get aligned from the specific version
    aligned_filepath = os.path.join(args.tmp_dir, "__tmp__id_" + run_id + "__aligned.fasta.xz")
    subprocess.run(["wget", aligned_version_download_link, "-O", aligned_filepath])
    curr_seq_strain = ""
    num_sequences_found = 0
    with lzma.open(aligned_filepath, "rt") as raw_data:
        for line in raw_data:
            if line.startswith(">"):
                curr_seq_strain = line[1:len(line) - 1]
                curr_seq_strain.strip()
            else:
                if curr_seq_strain in strain_to_aligned:
                    strain_to_aligned[curr_seq_strain] = line[:len(line) - 1].strip()
                    num_sequences_found += 1
                    if num_sequences_found == len(strain_to_aligned):
                        break
    os.remove(aligned_filepath)

    # Read the raw metadata one more time and get all the data to compose the JSON in required format.
    tmp_output = os.path.join(args.tmp_dir, "__tmp__id_" + run_id + "__output.json")
    file_output = open(tmp_output, "w")
    for chunk in pd.read_csv(metadata_raw, sep='\t', chunksize=chunksize):
        for _, row in chunk.iterrows():
            seq_json = {}
            strain = str(row['strain'])
            if (strain not in strain_to_aligned) or (strain_to_aligned[strain] == ""):
                continue
            seq_json["covv_accession_id"] = row["gisaid_epi_isl"]
            seq_json["covv_collection_date"] = row["date"]
            seq_json["covv_location"] = row["region"] + \
                " / " + row["country"] + " / " + row["division"]
            seq_json["sequence"] = strain_to_aligned[strain]
            seq_json["owner"] = row["authors"]
            seq_json["covv_host"] = row["host"]
            json.dump(seq_json, file_output)
            file_output.write("\n")
    file_output.close()
    os.remove(metadata_raw)

    # Copy the JSON content to the requested filepath.
    shutil.copy2(tmp_output, args.output)
    os.remove(tmp_output)

    print("\nThe final JSON file was written to '" + args.output + "'.")


if __name__ == "__main__":
    main(sys.argv[1:])
