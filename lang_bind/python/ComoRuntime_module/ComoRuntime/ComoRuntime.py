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

import como_pybind as ComoRuntime
import numpy as np
import multiprocessing
import sys
from itertools import chain

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class _ComoRuntime(object):
    """
    This class defines the API to inspect models and should not be used to
    create objects. It will be returned by functions such as load_model or
    train.

    In general this API assumes to be given only unicode for Python2 and the
    Python3 equvalent called str for any string-like arguments. All unicode
    strings are then encoded as UTF-8 and fed to the fastText C++ API.
    """

    def __init__(self, component_path=None, args=None):
        self.f = ComoRuntime.fasttext()
        if component_path is not None:
            self.f.loadModel(component_path)
        self._words = None
        self._labels = None
        self.set_args(args)

    def set_args(self, args=None):
        if args:
            arg_names = ['lr', 'dim']
            for arg_name in arg_names:
                setattr(self, arg_name, getattr(args, arg_name))

    def test(self, path, k=1):
        """Evaluate supervised model using file given by path"""
        return self.f.test(path, k)

    @property
    def words(self):
        if self._words is None:
            self._words = self.get_words()
        return self._words


def _build_args(args, manually_set_args):
    args["model"] = _parse_model_string(args["model"])
    args["loss"] = _parse_loss_string(args["loss"])
    if type(args["autotuneModelSize"]) == int:
        args["autotuneModelSize"] = str(args["autotuneModelSize"])

    a = ComoRuntime.args()
    for (k, v) in args.items():
        setattr(a, k, v)
        if k in manually_set_args:
            a.setManual(k)
    a.output = ""  # User should use save_model
    a.saveOutput = 0  # Never use this
    if a.wordNgrams <= 1 and a.maxn == 0:
        a.bucket = 0
    return a


def load_component(path):
    """Load a component given a filepath and return a Component object."""
    return _ComoRuntime(component_path=path)


unsupervised_default = {
    'model': "skipgram",
}


def read_args(arg_list, arg_dict, arg_names, default_values):
    ret = {}
    manually_set_args = set()
    for (arg_name, arg_value) in chain(zip(arg_names, arg_list), arg_dict.items()):
        if arg_name not in arg_names:
            raise TypeError("unexpected keyword argument '%s'" % arg_name)
        if arg_name in ret:
            raise TypeError("multiple values for argument '%s'" % arg_name)
        ret[arg_name] = arg_value
        manually_set_args.add(arg_name)

    for (arg_name, arg_value) in default_values.items():
        if arg_name not in ret:
            ret[arg_name] = arg_value

    return (ret, manually_set_args)


def train_supervised(*kargs, **kwargs):
    """
    Train a supervised model and return a model object.

    """
    supervised_default = unsupervised_default.copy()
    supervised_default.update({
        'lr': 0.1,
        'minCount': 1,
        'model': "supervised"
    })

    arg_names = ['input', 'lr', 'logFile']
    args, manually_set_args = read_args(kargs, kwargs, arg_names,
                                        supervised_default)
    a = _build_args(args, manually_set_args)
    ft = _ComoRuntime(args=a)
    ComoRuntime.train(ft.f, a)
    ft.set_args(ft.f.getArgs())
    return ft


