"""
Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

from ComoRuntime import load_model
from ComoRuntime import tokenize
import sys
import time
import tempfile
import argparse


def get_words(data, model):
    print("\nWords: " + str(t4 - t3))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Simple benchmark for get_words.')
    parser.add_argument('component', help='A COMO Component to use for benchmarking.')
    args = parser.parse_args()
    get_words(args.data, args.model)
