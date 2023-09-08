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
import sys

# This code assumes we are running from this directory
path_relative_to_git_root = "MenuSystem/"

plugins_to_process = ["MultiplayerSessions"]

# If files are passed in then restrict formatting to those
direct_matches = []
if 1 != len(sys.argv):
    for arg in sys.argv[1:]:
        direct_matches.append(arg.replace('\\', '/'))

try:
    changed_files = subprocess.check_output(["git", "diff", "--name-only", "HEAD", *direct_matches],
                                            universal_newlines=True).splitlines()

    project_changed_files = [x[len(path_relative_to_git_root):] for x in changed_files if x.startswith(path_relative_to_git_root)]

    files_to_format = []
    for file in project_changed_files:
        lower_filename = file.lower()
        if lower_filename.endswith(".h") or lower_filename.endswith(".cpp"):
            if file.startswith("Source/"):
                files_to_format.append(file)
            elif file.startswith("Plugins/"):
                for plugin in plugins_to_process:
                    if file.startswith(f"Plugins/{plugin}/Source/"):
                        files_to_format.append(file)

    if 0 != len(files_to_format):
        subprocess.run(["clang-format", "-i", *files_to_format])
        print("Formatted the following files:")
        for file in files_to_format:
            print(file)
except subprocess.CalledProcessError as e:
    print(f"Error executing git diff: {e}")
except Exception as e:
    print(f"An error occurred: {e}")
