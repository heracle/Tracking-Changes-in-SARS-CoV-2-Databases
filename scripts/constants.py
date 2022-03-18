#!/usr/bin/env python
import hashlib, json

LOOKUP_ALIGN_BASENAME = "lookup_align.json"

def get_hash (seq: str) -> str:
    return hashlib.sha256(str(seq).encode('utf-8')).hexdigest()

def get_hash_lookup(filepath: str) :
    f = open(filepath)
    data = json.load(f)
    return data
