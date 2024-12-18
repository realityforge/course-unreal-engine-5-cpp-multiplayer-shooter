#!/usr/bin/env python3

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
import argparse

projects_to_process = ["Blaster", "MenuSystem"]
plugins_to_process = ["RuleRanger", "MultiplayerSessions"]

parser = argparse.ArgumentParser(description="Unreal Source Code Formatter")

parser.add_argument('--verbose', action='store_true', help='Increase output verbosity')
parser.add_argument('files', type=str, nargs='*', help='The file to analyze')

args = parser.parse_args()

if args.verbose:
    print(f"Performing Source Code Formatting. Files: {args.files}")


try:
    index_files = subprocess.check_output(["git", "diff-index", "--cached", "--name-only", "HEAD", *args.files],
                                          universal_newlines=True).splitlines()
    tree_files = subprocess.check_output(["git", "ls-tree", "-r", "--name-only", "HEAD", *args.files],
                                    universal_newlines=True).splitlines()
    files = tree_files + index_files
    files_to_format = []
    files_to_format_assuming_json = []
    for file in files:
        for project in projects_to_process:
            if file.lower().endswith(".uplugin") or file.lower().endswith(".uproject"):
                files_to_format_assuming_json.append(file)
            elif file.lower().endswith(".h") or file.lower().endswith(".cpp") or file.lower().endswith(".cs"):
                if file.startswith(f"{project}/Source/"):
                    files_to_format.append(file)
                elif file.startswith(f"{project}/Plugins/"):
                    for plugin in plugins_to_process:
                        if file.startswith(f"{project}/Plugins/{plugin}/Source/"):
                            files_to_format.append(file)

    for file in files_to_format_assuming_json:
        content = ''
        with open(file, 'rb') as f:
            content = subprocess.run(["clang-format", f"--assume-filename={file}.json"], stdin=f, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True).stdout
        with open(file, 'w') as f:
            f.write(content)

    if 0 != len(files_to_format):
        subprocess.run(["clang-format", "-i", *files_to_format], check=True)

    if args.verbose:
        if 0 != len(files_to_format) or 0 != len(files_to_format_assuming_json):
            print("Formatted the following files:")
            for file in files_to_format_assuming_json:
                print(file)
            for file in files_to_format:
                print(file)

except subprocess.CalledProcessError as e:
    print(f"Error executing process: {e}")
    exit(e.returncode)
except Exception as e:
    print(f"An error occurred: {e}")
    exit(-1)
