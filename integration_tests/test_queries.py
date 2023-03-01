import unittest
import subprocess
from subprocess import PIPE
from tempfile import TemporaryDirectory
import glob
import os

"""Test queries """

CTC = './run'
TEST_DATA_DIR = os.path.dirname(os.path.realpath(__file__)) + '/../test_data/'

# TODO: create multiple test_query_<name>.py files, one for each different query.
class TestQueries(unittest.TestCase):
    def setUp(self):
        self.tempdir = TemporaryDirectory()

    def test_freq_bp(self):
        create_command = '{exe} create -o {outfile} {input}'.format(
            exe=CTC,
            outfile=self.tempdir.name + '/first',
            input=TEST_DATA_DIR + 'modified_sequence_v1.provision.json'
        )
        res = subprocess.run([create_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/first.h5',
            output_h5=self.tempdir.name + '/inter.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v2.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/inter.h5',
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v3.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 3
Size of 'data' field:12
Snapshot 'modified_sequence_v1.provision.json' contains 5 treap nodes.
Snapshot 'modified_sequence_v2.provision.json' contains 5 treap nodes.
Snapshot 'modified_sequence_v3.provision.json' contains 4 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------------------- query 0:  ""  without --compute-total-owner-cnt --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --num-to-print 34 --snapshot {snapshot} --exclude-deleted ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "": {
    "modified_sequence_v2.provision.json": {
      "bp0": {
        "bp index": 112,
        "number edits": 4,
        "number owners": 3,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>G",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "50%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>A",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "25%",
        },
        "owner2": {
          "number of edits": 1,
          "owner name": "ow1",
          "per bp distribution 0": {
            "kind": "A>G",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "25%",
        },
      },
      "bp1": {
        "bp index": 42,
        "number edits": 2,
        "number owners": 2,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
      },
      "bp2": {
        "bp index": 157,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp3": {
        "bp index": 177,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "list only top X bp": 34,
      "list only top X owners": 34,
      "owner0": {
        "name": "ow3",
        "number of edits": 2,
      },
      "owner1": {
        "name": "ow1",
        "number of edits": 1,
      },
      "owner2": {
        "name": "ow4",
        "number of edits": 1,
      },
    },
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 1:  ""  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --compute-total-owner-cnt --snapshot {snapshot} --exclude-deleted ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "": {
    "modified_sequence_v2.provision.json": {
      "bp0": {
        "bp index": 112,
        "number edits": 4,
        "number owners": 3,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>G",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "50%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>A",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "25%",
        },
        "owner2": {
          "number of edits": 1,
          "owner name": "ow1",
          "per bp distribution 0": {
            "kind": "A>G",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "25%",
        },
      },
      "bp1": {
        "bp index": 42,
        "number edits": 2,
        "number owners": 2,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
      },
      "bp2": {
        "bp index": 157,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp3": {
        "bp index": 177,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "list only top X bp": 50,
      "list only top X owners": 50,
      "owner0": {
        "name": "ow3",
        "number of edits": 2,
        "number of uploads": 2,
      },
      "owner1": {
        "name": "ow1",
        "number of edits": 1,
        "number of uploads": 1,
      },
      "owner2": {
        "name": "ow4",
        "number of edits": 1,
        "number of uploads": 1,
      },
    },
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 2:  "Oceania"  --------------------------------------------------------

        stats_command = '{exe} query --compute-total-owner-cnt -q bp_freq -i {input_h5} --snapshot {snapshot} --exclude-deleted "Oceania"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "Oceania": {
    "modified_sequence_v2.provision.json": {
      "bp0": {
        "bp index": 112,
        "number edits": 2,
        "number owners": 2,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow1",
          "per bp distribution 0": {
            "kind": "A>G",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
      },
      "bp1": {
        "bp index": 42,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp2": {
        "bp index": 157,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "list only top X bp": 50,
      "list only top X owners": 50,
      "owner0": {
        "name": "ow1",
        "number of edits": 1,
        "number of uploads": 1,
      },
      "owner1": {
        "name": "ow3",
        "number of edits": 1,
        "number of uploads": 1,
      },
    },
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 3:  "Europe"  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --compute-total-owner-cnt --snapshot {snapshot} --exclude-deleted "Europe"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "Europe": {
    "modified_sequence_v2.provision.json": {
      "bp0": {
        "bp index": 112,
        "number edits": 2,
        "number owners": 2,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>A",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>G",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "50%",
        },
      },
      "bp1": {
        "bp index": 42,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp2": {
        "bp index": 177,
        "number edits": 1,
        "number owners": 1,
        "owner0": {
          "number of edits": 1,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "A>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "list only top X bp": 50,
      "list only top X owners": 50,
      "owner0": {
        "name": "ow3",
        "number of edits": 1,
        "number of uploads": 1,
      },
      "owner1": {
        "name": "ow4",
        "number of edits": 1,
        "number of uploads": 1,
      },
    },
  },
},"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 4:  "America"  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} --snapshot {snapshot} --exclude-deleted "Void"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "Void": {
    "modified_sequence_v2.provision.json": {
      "list only top X bp": 50,
      "list only top X owners": 50,
    },
  },
},"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 5:  "" version2 vs version3  -----------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} --num-to-print 23 --exclude-deleted ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "": {
    "": {
      "bp0": {
        "bp index": 112,
        "number edits": 6,
        "number owners": 2,
        "owner0": {
          "number of edits": 4,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>G",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "per bp distribution 1": {
            "kind": "G>T",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "per bp distribution 2": {
            "kind": "C>A",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "per bp distribution 3": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "percent of current bp edits": "66%",
        },
        "owner1": {
          "number of edits": 2,
          "owner name": "ow1",
          "per bp distribution 0": {
            "kind": "G>A",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>G",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "33%",
        },
      },
      "bp1": {
        "bp index": 42,
        "number edits": 2,
        "number owners": 1,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>C",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "C>T",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp2": {
        "bp index": 157,
        "number edits": 2,
        "number owners": 1,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "C>A",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp3": {
        "bp index": 177,
        "number edits": 2,
        "number owners": 1,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>A",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>T",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "list only top X bp": 23,
      "list only top X owners": 23,
      "owner0": {
        "name": "ow3",
        "number of edits": 2,
        "number of uploads": 2,
      },
      "owner1": {
        "name": "ow1",
        "number of edits": 1,
        "number of uploads": 1,
      },
    },
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------------------- query 6:  "" version2 vs version3 including deleted sequences -----------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "": {
    "": {
      "bp0": {
        "bp index": 112,
        "number edits": 7,
        "number owners": 3,
        "owner0": {
          "number of edits": 4,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>G",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "per bp distribution 1": {
            "kind": "G>T",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "per bp distribution 2": {
            "kind": "C>A",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "per bp distribution 3": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "25%",
          },
          "percent of current bp edits": "57%",
        },
        "owner1": {
          "number of edits": 2,
          "owner name": "ow1",
          "per bp distribution 0": {
            "kind": "G>A",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>G",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "28%",
        },
        "owner2": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>A",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "14%",
        },
      },
      "bp1": {
        "bp index": 42,
        "number edits": 3,
        "number owners": 2,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>C",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "C>T",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "66%",
        },
        "owner1": {
          "number of edits": 1,
          "owner name": "ow4",
          "per bp distribution 0": {
            "kind": "G>T",
            "number of edits": 1,
            "percent of current owner edits": "100%",
          },
          "percent of current bp edits": "33%",
        },
      },
      "bp2": {
        "bp index": 157,
        "number edits": 2,
        "number owners": 1,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "C>A",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>C",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "bp3": {
        "bp index": 177,
        "number edits": 2,
        "number owners": 1,
        "owner0": {
          "number of edits": 2,
          "owner name": "ow3",
          "per bp distribution 0": {
            "kind": "T>A",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "per bp distribution 1": {
            "kind": "A>T",
            "number of edits": 1,
            "percent of current owner edits": "50%",
          },
          "percent of current bp edits": "100%",
        },
      },
      "list only top X bp": 50,
      "list only top X owners": 50,
      "owner0": {
        "name": "ow3",
        "number of edits": 2,
        "number of uploads": 2,
      },
      "owner1": {
        "name": "ow1",
        "number of edits": 1,
        "number of uploads": 1,
      },
      "owner2": {
        "name": "ow4",
        "number of edits": 1,
        "number of uploads": 1,
      },
    },
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

    def test_cnt_indels(self):
        create_command = '{exe} create -o {outfile} {input}'.format(
            exe=CTC,
            outfile=self.tempdir.name + '/first',
            input=TEST_DATA_DIR + 'modified_sequence_v1.provision.json'
        )
        res = subprocess.run([create_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/first.h5',
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v2.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            output_h5=self.tempdir.name + '/third.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v3.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 3
Size of 'data' field:12
Snapshot 'modified_sequence_v1.provision.json' contains 5 treap nodes.
Snapshot 'modified_sequence_v2.provision.json' contains 5 treap nodes.
Snapshot 'modified_sequence_v3.provision.json' contains 4 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # --------  Query 0 for cnt_indel for the third snapshot -----------------------------------------------------

        stats_command = '{exe} query -q cnt_indels -i {input_h5} --snapshot {snapshot} "" "Europe" "Oceania" "Void"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5',
            snapshot="modified_sequence_v1.provision.json,modified_sequence_v2.provision.json,modified_sequence_v3.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "": {
    "modified_sequence_v1.provision.json": {
      "inserted": 5,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 5,
      "modified": 4,
    },
    "modified_sequence_v3.provision.json": {
      "deleted": 1,
      "inserted": 4,
      "modified": 7,
    },
  },
},
{
  "Europe": {
    "modified_sequence_v1.provision.json": {
      "inserted": 2,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 2,
      "modified": 2,
    },
    "modified_sequence_v3.provision.json": {
      "deleted": 1,
      "inserted": 1,
      "modified": 3,
    },
  },
},
{
  "Oceania": {
    "modified_sequence_v1.provision.json": {
      "inserted": 3,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 3,
      "modified": 2,
    },
    "modified_sequence_v3.provision.json": {
      "inserted": 3,
      "modified": 4,
    },
  },
},
{
  "Void": {
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------- Query 1 for cnt_indel for the second snapshot (identical to the previous, but with shuffled queries) -----------------------------------------------------

        stats_command = '{exe} query -q cnt_indels -i {input_h5} --snapshot {snapshot} "Void" "Oceania" "Europe" "" '.format( # shuffle queries
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5',
            snapshot="modified_sequence_v1.provision.json,modified_sequence_v2.provision.json,modified_sequence_v3.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "Void": "",
},
{
  "Oceania": {
    "modified_sequence_v1.provision.json": {
      "inserted": 3,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 3,
      "modified": 2,
    },
    "modified_sequence_v3.provision.json": {
      "inserted": 3,
      "modified": 4,
    },
  },
},
{
  "Europe": {
    "modified_sequence_v1.provision.json": {
      "inserted": 2,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 2,
      "modified": 2,
    },
    "modified_sequence_v3.provision.json": {
      "deleted": 1,
      "inserted": 1,
      "modified": 3,
    },
  },
},
{
  "": {
    "modified_sequence_v1.provision.json": {
      "inserted": 5,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 5,
      "modified": 4,
    },
    "modified_sequence_v3.provision.json": {
      "deleted": 1,
      "inserted": 4,
      "modified": 7,
    },
  },
},"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------- Query 2 for cnt_indel for the second snapshot while excluding deletions -----------------------------------------------------

        stats_command = '{exe} query -q cnt_indels -i {input_h5} --snapshot {snapshot} --exclude-deleted "Void" "Oceania" "Europe" "" '.format( # shuffle queries
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5',
            snapshot="modified_sequence_v1.provision.json,modified_sequence_v2.provision.json,modified_sequence_v3.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)
        expected_stdout = """{
  "Void": "",
},
{
  "Oceania": {
    "modified_sequence_v1.provision.json": {
      "inserted": 3,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 3,
      "modified": 2,
    },
    "modified_sequence_v3.provision.json": {
      "inserted": 3,
      "modified": 4,
    },
  },
},
{
  "Europe": {
    "modified_sequence_v1.provision.json": {
      "inserted": 2,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 2,
      "modified": 2,
    },
    "modified_sequence_v3.provision.json": {
      "inserted": 1,
      "modified": 2,
    },
  },
},
{
  "": {
    "modified_sequence_v1.provision.json": {
      "inserted": 5,
      "modified": 0,
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 5,
      "modified": 4,
    },
    "modified_sequence_v3.provision.json": {
      "inserted": 4,
      "modified": 6,
    },
  },
},"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

    def test_cnt_hosts(self):
        create_command = '{exe} create -o {outfile} {input}'.format(
            exe=CTC,
            outfile=self.tempdir.name + '/first',
            input=TEST_DATA_DIR + 'cnt_host_test_v1.provision.json'
        )
        res = subprocess.run([create_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/first.h5',
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + 'cnt_host_test_v2.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 2
Size of 'data' field:5
Snapshot 'cnt_host_test_v1.provision.json' contains 3 treap nodes.
Snapshot 'cnt_host_test_v2.provision.json' contains 5 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # --------  Query 0 for cnt_indel for the third snapshot -----------------------------------------------------

        stats_command = '{exe} query -q cnt_hosts -i {input_h5} --snapshot {snapshot} "" "Europe" "Oceania" "Void"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="cnt_host_test_v1.provision.json,cnt_host_test_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """{
  "": {
    "cnt_host_test_v1.provision.json": {
      "Cat": 1,
      "Human": 2,
    },
    "cnt_host_test_v2.provision.json": {
      "Cat": 1,
      "Dog": 1,
      "Human": 3,
    },
  },
},
{
  "Europe": {
    "cnt_host_test_v2.provision.json": {
      "Dog": 1,
      "Human": 1,
    },
  },
},
{
  "Oceania": {
    "cnt_host_test_v1.provision.json": {
      "Cat": 1,
      "Human": 2,
    },
    "cnt_host_test_v2.provision.json": {
      "Cat": 1,
      "Human": 2,
    },
  },
},
{
  "Void": {
  },
},"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)
        