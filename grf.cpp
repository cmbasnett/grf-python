#include <iostream>
#include <Python.h>
#include <grf.h>

typedef struct {
    PyObject_HEAD
    Grf * grf;
    // TODO: we have to make sure it can't be copied!
} grf_ArchiveObject;

typedef struct {
    PyObject_HEAD
    GrfFile file;
} grf_GrfFileObject;

static PyTypeObject grf_ArchiveType = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "grf.Archive",
        sizeof(grf_ArchiveObject),
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
};

static PyTypeObject grf_GrfFileType = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "grf.GrfFile",
        sizeof(grf_GrfFileObject),
};

static void grf_Archive_dealloc(PyObject *self) {
    auto grf = reinterpret_cast<grf_ArchiveObject *>(self)->grf;
    grf_close(grf);
//    grf_free(grf);
}

static int grf_Archive_init(PyObject *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {(char*)"path", (char*)"mode", nullptr};
    const char* path = nullptr;
    const char* mode = "r";
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, &path, &mode)) {
        return -1;
    }
    auto archive = reinterpret_cast<grf_ArchiveObject *>(self);
    GrfError error;
    archive->grf = grf_open(path, "r", &error);
    if (archive->grf == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, grf_strerror(error));
        return -1;
    }
    return 0;
}

static PyObject *grf_Archive_get_filename(PyObject *self, void *closure) {
    auto grf = reinterpret_cast<grf_ArchiveObject *>(self)->grf;
    const char* errors;
    return PyUnicode_Decode(grf->filename, strlen(grf->filename), "utf-8", errors);
}

static PyObject *grf_Archive_get_nfiles(PyObject *self, void *closure) {
    auto grf = reinterpret_cast<grf_ArchiveObject *>(self)->grf;
    return PyLong_FromUnsignedLong(grf->nfiles);
}

static PyObject *grf_Archive_get_version(PyObject *self, void *closure) {
    auto grf = reinterpret_cast<grf_ArchiveObject *>(self)->grf;
    return PyLong_FromUnsignedLong(grf->version);
}

static PyObject *grf_Archive_get_files(PyObject *self, void *closure) {
    // TODO: very expensive!
    auto grf = reinterpret_cast<grf_ArchiveObject *>(self)->grf;
    PyObject* files = PyList_New(0);
    for (size_t i = 0; i < grf->nfiles; ++i) {
        if ((grf->files[i].flags & GRFFILE_FLAG_FILE) == 0) {
            continue;
        }
        auto grf_file = reinterpret_cast<grf_GrfFileObject*>(PyObject_New(grf_GrfFileObject, &grf_GrfFileType));
        grf_file->file = grf->files[i];
        PyList_Append(files, reinterpret_cast<PyObject*>(grf_file));
    }
    return files;
}

static struct PyGetSetDef grf_Archive_getset[] = {
        {"filename", reinterpret_cast<getter>(grf_Archive_get_filename), nullptr, nullptr, nullptr},
        {"nfiles", reinterpret_cast<getter>(grf_Archive_get_nfiles), nullptr, nullptr, nullptr},
        {"version", reinterpret_cast<getter>(grf_Archive_get_version), nullptr, nullptr, nullptr},
        {"files", reinterpret_cast<getter>(grf_Archive_get_files), nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr}
};

static PyObject *GrfErr_GetException(GrfError error) {
    switch (error.type) {
        case GE_BADARGS:
            return PyExc_ValueError;
        case GE_INVALID:
        case GE_CORRUPTED:
        case GE_BADMODE:
        case GE_ZLIB:
        case GE_ZLIBFILE:
            return PyExc_IOError;
        case GE_NSUP:
        case GE_NOTIMPLEMENTED:
            return PyExc_NotImplementedError;
        case GE_NOTFOUND:
            return PyExc_LookupError;
        case GE_INDEX:
            return PyExc_IndexError;
        default:
            return PyExc_RuntimeError;
    }
}

static PyObject *grf_Archive_extract(PyObject *self, PyObject *args, PyObject *kwds) {
    auto grf = reinterpret_cast<grf_ArchiveObject *>(self)->grf;
    const char *grfname = nullptr;
    const char *file = nullptr;
    static char *kwlist[] = {(char*)"grfname", (char*)"file", nullptr};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ys", kwlist, &grfname, &file)) {
        return nullptr;
    }
    GrfError error;
    grf_extract(grf, grfname, file, &error);
    if (error.type != GE_SUCCESS && error.type != GE_NODATA) {
        PyErr_SetString(GrfErr_GetException(error), grf_strerror(error));
        return nullptr;
    }
    Py_RETURN_NONE;
}

static PyMethodDef grf_Archive_methods[] = {
    { "extract", reinterpret_cast<PyCFunction>(grf_Archive_extract), METH_VARARGS | METH_KEYWORDS, nullptr },
    {nullptr}
};

static PyObject *grf_GrfFile_get_name(PyObject *self, void *closure) {
    auto file = reinterpret_cast<grf_GrfFileObject *>(self);
    return PyBytes_FromString(file->file.name);
}

static PyObject *grf_GrfFile_get_real_len(PyObject *self, void *closure) {
    auto file = reinterpret_cast<grf_GrfFileObject *>(self);
    return PyLong_FromUnsignedLong(file->file.real_len);
}

static PyObject *grf_GrfFile_get_compressed_len(PyObject *self, void *closure) {
    auto file = reinterpret_cast<grf_GrfFileObject *>(self);
    return PyLong_FromUnsignedLong(file->file.compressed_len);
}

static PyGetSetDef grf_GrfFile_getset[] = {
        {"name", reinterpret_cast<getter>(grf_GrfFile_get_name), nullptr, nullptr, nullptr},
        {"real_len", reinterpret_cast<getter>(grf_GrfFile_get_real_len), nullptr, nullptr, nullptr},
        {"compressed_len", reinterpret_cast<getter>(grf_GrfFile_get_compressed_len), nullptr, nullptr, nullptr},
        {nullptr}
};

static PyMethodDef grf_methods[] = {
        { nullptr }
};

static struct PyModuleDef grf_definition = {
        PyModuleDef_HEAD_INIT,
        "grf",
        "libgrf",
        -1,
        grf_methods
};

PyMODINIT_FUNC PyInit_grf(void) {
    grf_ArchiveType.tp_new = PyType_GenericNew;
    grf_ArchiveType.tp_dealloc = grf_Archive_dealloc;
    grf_ArchiveType.tp_init = grf_Archive_init;
    grf_ArchiveType.tp_getset = grf_Archive_getset;
    grf_ArchiveType.tp_flags = Py_TPFLAGS_DEFAULT;
    grf_ArchiveType.tp_doc = "GRF archive";
    grf_ArchiveType.tp_methods = grf_Archive_methods;

    grf_GrfFileType.tp_new = nullptr;
    grf_GrfFileType.tp_getset = grf_GrfFile_getset;

    if (PyType_Ready(&grf_ArchiveType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&grf_GrfFileType) < 0) {
        return nullptr;
    }

    PyObject* module = PyModule_Create(&grf_definition);

    if (module == nullptr) {
        return nullptr;
    }

    Py_INCREF(&grf_ArchiveType);
    PyModule_AddObject(module, "Archive", reinterpret_cast<PyObject *>(&grf_ArchiveType));

    Py_INCREF(&grf_GrfFileType);
    PyModule_AddObject(module, "GrfFile", reinterpret_cast<PyObject *>(&grf_GrfFileType));

    return module;
}
