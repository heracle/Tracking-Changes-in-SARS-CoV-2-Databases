import unittest
import subprocess
from subprocess import PIPE
from tempfile import TemporaryDirectory
import glob
import os

"""Test ctc create"""

CTC = './run'
TEST_DATA_DIR = os.path.dirname(os.path.realpath(__file__)) + '/../test_data/'

class TestCli(unittest.TestCase):
    def setUp(self):
        self.tempdir = TemporaryDirectory()

    def test_create_append_stats_cli(self):
        create_command = '{exe} create -o {outfile} {input}'.format(
            exe=CTC,
            outfile=self.tempdir.name + '/first',
            input=TEST_DATA_DIR + '0000-01-01.provision.json'
        )
        res = subprocess.run([create_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/first.h5',
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + '0000-01-01_with_2_changes.provision.json ' + TEST_DATA_DIR + '0000-01-01_with_4_changes.provision.json ' + TEST_DATA_DIR + '0000-01-01_with_6_changes.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 4
Size of 'data' field:24
Snapshot '0000-01-01.provision.json' contains 20 treap nodes.
Snapshot '0000-01-01_with_2_changes.provision.json' contains 20 treap nodes.
Snapshot '0000-01-01_with_4_changes.provision.json' contains 18 treap nodes.
Snapshot '0000-01-01_with_6_changes.provision.json' contains 20 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)
