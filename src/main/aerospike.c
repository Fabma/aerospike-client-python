/*******************************************************************************
 * Copyright 2013-2015 Aerospike, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include <Python.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "client.h"
#include "key.h"
#include "query.h"
#include "geo.h"
#include "scan.h"
#include "predicates.h"
#include "exceptions.h"
#include "lstack.h"
#include "lset.h"
#include "llist.h"
#include "lmap.h"
#include "policy.h"
#include "log.h"
#include <aerospike/as_operations.h>
#include "serializer.h"
#include "nullobject.h"

PyObject *py_global_hosts;
int counter = 0xA5000000;
bool user_shm_key = false;

static PyMethodDef Aerospike_Methods[] = {

	//Serializer Operations

	{"set_serializer",
		(PyCFunction)AerospikeClient_Set_Serializer, METH_VARARGS | METH_KEYWORDS,
		"Sets the serializer"},
	{"set_deserializer",
		(PyCFunction)AerospikeClient_Set_Deserializer, METH_VARARGS | METH_KEYWORDS,
		"Sets the deserializer"},
	{"unset_serializers",
		(PyCFunction)AerospikeClient_Unset_Serializers, METH_VARARGS | METH_KEYWORDS,
		"Unsets the serializer and deserializer"},
	{"client",		(PyCFunction) AerospikeClient_New,	METH_VARARGS | METH_KEYWORDS,
		"Create a new instance of Client class."},
	{"set_log_level",	(PyCFunction)Aerospike_Set_Log_Level,       METH_VARARGS | METH_KEYWORDS,
		"Sets the log level"},
	{"set_log_handler", (PyCFunction)Aerospike_Set_Log_Handler,	    METH_VARARGS | METH_KEYWORDS,
		"Sets the log handler"},
	{"geodata", (PyCFunction)Aerospike_Set_Geo_Data,	    METH_VARARGS | METH_KEYWORDS,
		"Creates a GeoJSON object from geospatial data."},
	{"geojson", (PyCFunction)Aerospike_Set_Geo_Json,	    METH_VARARGS | METH_KEYWORDS,
		"Creates a GeoJSON object from a raw GeoJSON string."},
	{NULL}
};

static
AerospikeConstants operator_constants[] = {
	{ AS_OPERATOR_READ                 ,   "OPERATOR_READ"    },
	{ AS_OPERATOR_WRITE                ,   "OPERATOR_WRITE"   },
	{ AS_OPERATOR_INCR                 ,   "OPERATOR_INCR"    },
	{ AS_OPERATOR_APPEND               ,   "OPERATOR_APPEND"  },
	{ AS_OPERATOR_PREPEND              ,   "OPERATOR_PREPEND" },
	{ AS_OPERATOR_TOUCH                ,   "OPERATOR_TOUCH"   }
};

#define OPERATOR_CONSTANTS_ARR_SIZE (sizeof(operator_constants)/sizeof(AerospikeConstants))

MODULE_INIT_FUNC(aerospike)
{
	static char version[6] = "1.0.60";
	// Makes things "thread-safe"
	PyEval_InitThreads();
	int i = 0;
    
	static struct PyModuleDef moduledef = { 
			PyModuleDef_HEAD_INIT, /* m_base */
			"aerospike", /* m_name */
			"Aerospike Python Client", /* m_doc */
			-1, /* m_size */
			Aerospike_Methods, /* m_methods */
	};
	PyObject * aerospike = PyModule_Create(&moduledef);

	if(aerospike == NULL)
    {
        return NULL;
    }

	py_global_hosts = PyDict_New();
	declare_policy_constants(aerospike);

	PyModule_AddStringConstant(aerospike, "__version__", version);

	PyObject * exception = AerospikeException_New();
	Py_INCREF(exception);
	PyModule_AddObject(aerospike, "exception", exception);

	PyTypeObject * client = AerospikeClient_Ready();
	Py_INCREF(client);
	PyModule_AddObject(aerospike, "Client", (PyObject *) client);

	PyTypeObject * key = AerospikeKey_Ready();
	Py_INCREF(key);
	PyModule_AddObject(aerospike, "Key", (PyObject *) key);

	PyTypeObject * query = AerospikeQuery_Ready();
	Py_INCREF(query);
	PyModule_AddObject(aerospike, "Query", (PyObject *) query);

	declare_policy_constants(aerospike);
	declare_log_constants(aerospike);

	PyTypeObject * scan = AerospikeScan_Ready();
	Py_INCREF(scan);
	PyModule_AddObject(aerospike, "Scan", (PyObject *) scan);

	for (i = 0; i <= OPERATOR_CONSTANTS_ARR_SIZE; i++) {
		PyModule_AddIntConstant(aerospike,
				operator_constants[i].constant_str,
				operator_constants[i].constantno);
	}

	/*
	 * Add constants to module.
	 */
	declare_policy_constants(aerospike);

	PyObject * predicates = AerospikePredicates_New();
	Py_INCREF(predicates);
	PyModule_AddObject(aerospike, "predicates", predicates);


	PyTypeObject * lstack = AerospikeLStack_Ready();
	Py_INCREF(lstack);
	PyModule_AddObject(aerospike, "lstack", (PyObject *) lstack);

	PyTypeObject * lset = AerospikeLSet_Ready();
	Py_INCREF(lset);
	PyModule_AddObject(aerospike, "lset", (PyObject *) lset);

	PyTypeObject * llist = AerospikeLList_Ready();
	Py_INCREF(llist);
	PyModule_AddObject(aerospike, "llist", (PyObject *) llist);

	PyTypeObject * lmap = AerospikeLMap_Ready();
	Py_INCREF(lmap);
	PyModule_AddObject(aerospike, "lmap", (PyObject *) lmap);

	PyTypeObject * geospatial = AerospikeGeospatial_Ready();
	Py_INCREF(geospatial);
	PyModule_AddObject(aerospike, "GeoJSON", (PyObject *) geospatial);

	PyObject * null_object = AerospikeNullObject_New();
	Py_INCREF(null_object);
	PyModule_AddObject(aerospike, "null", (PyObject *) null_object);

    return aerospike;
}
