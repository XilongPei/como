//=========================================================================
// Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include <args.h>
#include <autotune.h>
#include <densematrix.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <real.h>
#include <vector.h>
#include <cmath>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <comoapi.h>

using namespace pybind11::literals;
namespace py = pybind11;

py::str castToPythonString(const std::string& s, const char* onUnicodeError) {
  PyObject* handle = PyUnicode_DecodeUTF8(s.data(), s.length(), onUnicodeError);
  if (!handle) {
    throw py::error_already_set();
  }

  // py::str's constructor from a PyObject assumes the string has been encoded
  // for python 2 and not encoded for python 3 :
  // https://github.com/pybind/pybind11/blob/ccbe68b084806dece5863437a7dc93de20bd9b15/include/pybind11/pytypes.h#L930
#if PY_MAJOR_VERSION < 3
  PyObject* handle_encoded =
      PyUnicode_AsEncodedString(handle, "utf-8", onUnicodeError);
  Py_DECREF(handle);
  handle = handle_encoded;
#endif

  py::str handle_str = py::str(handle);
  Py_DECREF(handle);
  return handle_str;
}

PYBIND11_MODULE(como_pybind, m) {
  py::class_<como::Args>(m, "args")
      .def(py::init<>())
      .def_readwrite("input", &como::Args::input)
      .def_readwrite("output", &como::Args::output)
      .def("setManual", [](como::Args& m, const std::string& argName) {
        m.setManual(argName);
      });

  py::enum_<como::model_name>(m, "model_name")
      .value("cbow", como::model_name::cbow)
      .value("skipgram", como::model_name::sg)
      .value("supervised", como::model_name::sup)
      .export_values();

  m.def(
      "train",
      [](como::ComoRuntime& ft, como::Args& a) {
        if (a.hasAutotune()) {
          como::Autotune autotune(std::shared_ptr<como::ComoRuntime>(
              &ft, [](como::ComoRuntime*) {}));
          autotune.train(a);
        } else {
          ft.train(a);
        }
      },
      py::call_guard<py::gil_scoped_release>());
      .def(
          "loadModel",
          [](como::ComoRuntime& m, std::string s) { m.loadModel(s); })

  py::class_<como::ComoRuntime>(m, "ComoRuntime")
      .def(py::init<>())
      .def("getArgs", &como::como::getArgs)
      .def(
          "getInputMatrix",
          [](como::ComoRuntime& m) {
            std::shared_ptr<const como::DenseMatrix> mm =
                m.getInputMatrix();
            return mm.get();
          },
          pybind11::return_value_policy::reference)
      .def(
          "getOutputMatrix",
          [](como::ComoRuntime& m) {
            std::shared_ptr<const como::DenseMatrix> mm =
                m.getOutputMatrix();
            return mm.get();
          },
          pybind11::return_value_policy::reference)
      .def(
          "setMatrices",
          [](como::ComoRuntime& m,
             py::buffer inputMatrixBuffer,
             py::buffer outputMatrixBuffer) {
            py::buffer_info inputMatrixInfo = inputMatrixBuffer.request();
            py::buffer_info outputMatrixInfo = outputMatrixBuffer.request();

            m.setMatrices(
                std::make_shared<como::DenseMatrix>(
                    inputMatrixInfo.shape[0],
                    inputMatrixInfo.shape[1],
                    static_cast<float*>(inputMatrixInfo.ptr)),
                std::make_shared<como::DenseMatrix>(
                    outputMatrixInfo.shape[0],
                    outputMatrixInfo.shape[1],
                    static_cast<float*>(outputMatrixInfo.ptr)));
          })
      .def(
          "loadModel",
          [](como::ComoRuntime& m, std::string s) { m.loadModel(s); })
}
