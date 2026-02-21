import sys
import subprocess
import argparse
from pathlib import Path

#python RunTests.py --path="./bin/Engine/Debug/x64/Tests"

parser = argparse.ArgumentParser("TestRunner")
parser.add_argument("-p", "--path",
                    help="Path to folder with tests to run.",
                    type=Path, required=True)
args = parser.parse_args();

if not args.path.exists():
    raise ValueError(f"Path {args.path} does not exist.")

print(f"Running tests in {args.path}")

testFilesList = args.path.glob('*Test.exe')

errors = sum(
    subprocess.run([testFile], cwd=args.path,
                   stdout=sys.stdout,
                   stderr=sys.stderr).returncode
    for testFile in testFilesList
)

exit(errors)