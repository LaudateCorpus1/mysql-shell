/*
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef _PYTHON_TYPE_CONVERSION_H_
#define _PYTHON_TYPE_CONVERSION_H_

#include "shellcore/types.h"
#include <Python.h>

namespace shcore {

class Python_context;

struct Python_type_bridger
{
  Python_type_bridger(Python_context *context);
  ~Python_type_bridger();

  void init();

  Value pyobj_to_shcore_value(PyObject *value) const;
  PyObject *shcore_value_to_pyobj(const Value &value);

  PyObject *native_object_to_py(Object_bridge_ref object);

  Python_context *_owner;
  class Python_array_wrapper *_array_wrapper;
  class Python_map_wrapper *_map_wrapper;

  /*
  class Python_object_wrapper *_object_wrapper;
  class Python_function_wrapper *_function_wraper;
  */
};

}

#endif
