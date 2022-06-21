#!/usr/bin/env python
import hashlib, json

LOOKUP_ALIGN_FILEPATH = "/cluster/scratch/<user>/gisaid_data/lookup_2022-03-13.json"
NEXTALIGN_EXE_PATH = "<TCVD git root>/src/external_libraries/nextclade-Linux-x86_64"
COVID_NEXTALIGN_DATASET = "/cluster/scratch/<user>/gisaid_data/ref_sars-cov-2/"

def get_hash (seq: str) -> str:
    return hashlib.sha256(str(seq).encode('utf-8')).hexdigest()

def get_hash_lookup(filepath: str) :
    f = open(filepath)
    data = json.load(f)
    return data
